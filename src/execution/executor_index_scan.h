/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "errors.h"
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class IndexScanExecutor : public AbstractExecutor {
   private:
    std::string tab_name_;  // 表名称
    TabMeta tab_;           // 表的元数据
    std::vector<Condition>
        conds_;  // 扫描条件，所有conds的左、右列中都必有一列是表tab_name_中的索引，要保证conds中的顺序符合索引的顺序，目前只支持右列是value，要保证cond均被完全初始化(init_raw)
    RmFileHandle *fh_;                  // 表的数据文件句柄
    std::vector<ColMeta> cols_;         // 需要读取的表上的所有字段
    size_t len_;                        // 选取出来的一条记录的长度
    std::vector<Condition> idx_conds_;  // 真正使用索引进行查找的条件

    std::vector<std::string> index_col_names_;  // index scan涉及到的索引包含的字段
    IndexMeta index_meta_;                      // index scan涉及到的索引元数据

    Rid rid_;
    char *key_;
    std::unique_ptr<RecScan> scan_;

    SmManager *sm_manager_;

   public:
    IndexScanExecutor(SmManager *sm_manager, std::string tab_name, std::vector<Condition> conds,
                      std::vector<Condition> idx_conds, std::vector<std::string> index_col_names, Context *context) {
        sm_manager_ = sm_manager;
        context_ = context;
        tab_name_ = std::move(tab_name);

        sm_manager_->db_.RLatch();
        tab_ = sm_manager_->db_.get_table(tab_name_);
        sm_manager_->db_.RUnLatch();

        conds_ = std::move(conds);
        // index_no_ = index_no;
        index_col_names_ = index_col_names;
        index_meta_ = *(tab_.get_index_meta(index_col_names_));
        sm_manager_->latch_.lock_shared();
        fh_ = sm_manager_->fhs_.at(tab_name_).get();
        sm_manager_->latch_.unlock_shared();
        cols_ = tab_.cols;
        len_ = cols_.back().offset + cols_.back().len;
        std::map<CompOp, CompOp> swap_op = {
            {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
        };

        for (auto &cond : conds_) {
            if (cond.lhs_col.tab_name != tab_name_) {
                // TRY:我认为目前不可能出现这种情况
                assert(false);
                // lhs is on other table, now rhs must be on this table
                assert(!cond.is_rhs_val && cond.rhs_col.tab_name == tab_name_);
                // swap lhs and rhs
                std::swap(cond.lhs_col, cond.rhs_col);
                cond.op = swap_op.at(cond.op);
            }
        }
        idx_conds_ = std::move(idx_conds);

        key_ = new char[index_meta_.col_tot_len];
        /*
        // make key
        int offset = 0;
        // 目前只支持右边是value的情况
        for (auto &cond : conds_) {
            std::shared_ptr<RmRecord> raw_data = cond.rhs_val.raw;
            memcpy(key_ + offset, raw_data->data, raw_data->size);
            offset += raw_data->size;
        }*/
    }

    void beginTuple() override {
        if (context_->lock_mgr_->lock_shared_on_table(context_->txn_, fh_->GetFd()) == false) {
            throw TransactionAbortException(context_->txn_->get_transaction_id(), AbortReason::GET_LOCK_FAILED);
        }

        IxManager *ix_manager = sm_manager_->get_ix_manager();
        sm_manager_->latch_.lock_shared();
        IxIndexHandle *ix_hdl = sm_manager_->ihs_.at(ix_manager->get_index_name(tab_name_, index_col_names_)).get();
        sm_manager_->latch_.unlock_shared();

        Iid def_lower_bound = ix_hdl->leaf_begin(), def_upper_bound = ix_hdl->leaf_end();

        size_t first_range_cond = 0;
        int offset = 0;
        while (first_range_cond < idx_conds_.size() && idx_conds_[first_range_cond].op == CompOp::OP_EQ) {
            std::shared_ptr<RmRecord> raw_data = idx_conds_[first_range_cond].rhs_val.raw;
            memcpy(key_ + offset, raw_data->data, raw_data->size);
            offset += raw_data->size;
            ++first_range_cond;
        }
        size_t i = first_range_cond;
        if (i == idx_conds_.size()) {
            // 等于条件时认为列名一定只出现一次，故参与比较的key恰好是前i个col
            def_lower_bound = ix_hdl->lower_bound(key_, i);
            def_upper_bound = ix_hdl->upper_bound(key_, i);
        } else {
            Value lower_val, upper_val;
            bool le = false, be = false;
            bool has_lower = false, has_upper = false;
            std::string col_name = idx_conds_[i].lhs_col.col_name;
            while (i < idx_conds_.size() && idx_conds_[i].lhs_col.col_name.compare(col_name) == 0) {
                if (idx_conds_[i].op == CompOp::OP_GE || idx_conds_[i].op == CompOp::OP_GT) {
                    // 当存在更高的下限时，更新旧的下限
                    if (!has_lower || idx_conds_[i].rhs_val > lower_val ||
                        (idx_conds_[i].rhs_val == lower_val && be && idx_conds_[i].op == CompOp::OP_GT)) {
                        lower_val = idx_conds_[i].rhs_val;
                        be = (idx_conds_[i].op == CompOp::OP_GE);
                    }
                    has_lower = true;
                } else if (idx_conds_[i].op == CompOp::OP_LE || idx_conds_[i].op == CompOp::OP_LT) {
                    if (!has_upper || idx_conds_[i].rhs_val < upper_val ||
                        (idx_conds_[i].rhs_val == upper_val && le && idx_conds_[i].op == CompOp::OP_LT)) {
                        upper_val = idx_conds_[i].rhs_val;
                        le = (idx_conds_[i].op == CompOp::OP_LE);
                    }
                    has_upper = true;
                }
                ++i;
            }
            // 下界大于上界，直接结束
            if (has_lower && has_upper && (lower_val > upper_val || (lower_val == upper_val && !(be && le)))) {
                scan_ = std::make_unique<IxScan>(ix_hdl, Iid{0, 0}, Iid{0, 0}, sm_manager_->get_bpm(), true);
            } else {
                if (has_lower) {
                    std::shared_ptr<RmRecord> raw_data = lower_val.raw;
                    memcpy(key_ + offset, raw_data->data, raw_data->size);
                    def_lower_bound = be ? ix_hdl->lower_bound(key_, first_range_cond + 1)
                                         : ix_hdl->upper_bound(key_, first_range_cond + 1);
                } else if (first_range_cond != 0) {
                    // 没有下界且前面有等于条件时，按照前面的等于条件设置下界
                    def_lower_bound = ix_hdl->lower_bound(key_, first_range_cond);
                }  // 否则下界设置为初始值(leaf_begin)

                if (has_upper) {
                    std::shared_ptr<RmRecord> raw_data = upper_val.raw;
                    memcpy(key_ + offset, raw_data->data, raw_data->size);
                    def_upper_bound = le ? ix_hdl->upper_bound(key_, first_range_cond + 1)
                                         : ix_hdl->lower_bound(key_, first_range_cond + 1);
                } else if (first_range_cond != 0) {
                    // 没有上界且前面有等于条件时，按照前面的等于条件设置上界
                    def_upper_bound = ix_hdl->upper_bound(key_, first_range_cond);
                }  // 否则上界设置为初始值(leaf_end)}
            }
        }
        scan_ = std::make_unique<IxScan>(ix_hdl, def_lower_bound, def_upper_bound, sm_manager_->get_bpm());

        for (auto rid = scan_->rid(); !scan_->is_end(); scan_->next()) {
            rid = scan_->rid();
            auto record = fh_->get_record(rid, context_);
            bool cond_flag = true;
            // test conds
            for (auto &cond : conds_) {
                cond_flag = cond_flag && cond.test_record(tab_.cols, record);
                if (!cond_flag) {
                    break;
                }
            }
            if (cond_flag) {
                rid_ = rid;

                return;
            }
        }
    }

    void nextTuple() override {
        Rid rid;
        scan_->next();
        while (!scan_->is_end()) {
            rid = scan_->rid();
            auto record = fh_->get_record(rid, context_);
            bool cond_flag = true;
            // test conds
            for (auto &cond : conds_) {
                cond_flag = cond_flag && cond.test_record(tab_.cols, record);
                if (!cond_flag) {
                    break;
                }
            }
            if (cond_flag) {
                rid_ = rid;
                return;
            }

            scan_->next();
        }
    }

    bool is_end() const override {
        bool ret = scan_->is_end();
        return ret;
    }

    virtual const std::vector<ColMeta> &cols() const { return cols_; }

    std::unique_ptr<RmRecord> Next() override { return fh_->get_record(rid_, context_); }

    std::string getType() override { return "IndexScanExecutor"; }

    int tupleLen() const override { throw UnreachableCodeError(); }

    Rid &rid() override { return rid_; }

    ~IndexScanExecutor() { delete[] key_; }
};
