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
        conds_;  // 扫描条件，所有conds的左、右列中都必有一列是表tab_name_中的索引，要保证conds中的顺序符合索引的顺序，目前只支持其中一列是value，要保证cond均被完全初始化(init_raw)
    RmFileHandle *fh_;           // 表的数据文件句柄
    std::vector<ColMeta> cols_;  // 需要读取的字段
    size_t len_;                 // 选取出来的一条记录的长度
    std::vector<Condition>
        fed_conds_;  // 扫描条件，和conds_字段相同，所有的conds列都是有索引的

    std::vector<std::string>
        index_col_names_;   // index scan涉及到的索引包含的字段
    IndexMeta index_meta_;  // index scan涉及到的索引元数据

    Rid rid_;
    char *key_;
    std::unique_ptr<RecScan> scan_;

    SmManager *sm_manager_;

   public:
    IndexScanExecutor(SmManager *sm_manager, std::string tab_name,
                      std::vector<Condition> conds,
                      std::vector<std::string> index_col_names,
                      Context *context) {
        sm_manager_ = sm_manager;
        context_ = context;
        tab_name_ = std::move(tab_name);
        tab_ = sm_manager_->db_.get_table(tab_name_);
        conds_ = std::move(conds);
        // index_no_ = index_no;
        index_col_names_ = index_col_names;
        index_meta_ = *(tab_.get_index_meta(index_col_names_));
        fh_ = sm_manager_->fhs_.at(tab_name_).get();
        cols_ = tab_.cols;
        len_ = cols_.back().offset + cols_.back().len;
        std::map<CompOp, CompOp> swap_op = {
            {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT},
            {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
        };

        for (auto &cond : conds_) {
            if (cond.lhs_col.tab_name != tab_name_) {
                // lhs is on other table, now rhs must be on this table
                assert(!cond.is_rhs_val && cond.rhs_col.tab_name == tab_name_);
                // swap lhs and rhs
                std::swap(cond.lhs_col, cond.rhs_col);
                cond.op = swap_op.at(cond.op);
            }
        }
        fed_conds_ = conds_;

        key_ = new char[index_meta_.col_tot_len];
        int offset = 0;
        // 目前只支持右边是value的情况
        for (auto &cond : conds_) {
            std::shared_ptr<RmRecord> raw_data = cond.rhs_val.raw;
            memcpy(key_ + offset, raw_data->data, raw_data->size);
            offset += raw_data->size;
        }
    }

    void beginTuple() override {
        IxManager *ix_manager = sm_manager_->get_ix_manager();
        IxIndexHandle *ix_hdl =
            sm_manager_->ihs_
                .at(ix_manager->get_index_name(tab_name_, index_col_names_))
                .get();

        Iid def_lower_bound = ix_hdl->leaf_begin(),
            def_upper_bound = ix_hdl->leaf_end();

        for (size_t i = 0; i < conds_.size(); ++i) {
            if (conds_[i].op == OP_EQ) {
                def_lower_bound = ix_hdl->lower_bound(key_, i);
                def_upper_bound = ix_hdl->upper_bound(key_, i);
            } else if (conds_[i].op == OP_GE) {
                scan_ = std::make_unique<IxScan>(
                    ix_hdl, ix_hdl->lower_bound(key_, i), def_upper_bound,
                    sm_manager_->get_bpm());
            } else if (conds_[i].op == OP_GT) {
                scan_ = std::make_unique<IxScan>(
                    ix_hdl, ix_hdl->upper_bound(key_, i), def_upper_bound,
                    sm_manager_->get_bpm());
            } else if (conds_[i].op == OP_LE) {
                scan_ = std::make_unique<IxScan>(ix_hdl, def_lower_bound,
                                                 ix_hdl->upper_bound(key_, i),
                                                 sm_manager_->get_bpm());
            } else if (conds_[i].op == OP_LT) {
                scan_ = std::make_unique<IxScan>(ix_hdl, def_lower_bound,
                                                 ix_hdl->lower_bound(key_, i),
                                                 sm_manager_->get_bpm());
            } else if (conds_[i].op == OP_NE) {
                scan_ = std::make_unique<IxScan>(ix_hdl, def_lower_bound,
                                                 def_upper_bound,
                                                 sm_manager_->get_bpm());
            }
        }

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

    std::unique_ptr<RmRecord> Next() override { return nullptr; }

    Rid &rid() override { return rid_; }

    ~IndexScanExecutor() { delete[] key_; }
};