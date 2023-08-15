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

#include <memory>

#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "record/rm_scan.h"
#include "system/sm.h"
#include "system/sm_meta.h"
#include "transaction/txn_defs.h"

class SeqScanExecutor : public AbstractExecutor {
   private:
    std::string tab_name_;              // 表的名称
    TabMeta tab_;
    std::vector<Condition> conds_;      // scan的条件
    RmFileHandle *fh_;                  // 表的数据文件句柄
    std::vector<ColMeta> cols_;         // scan后生成的记录的字段
    size_t len_;                        // scan后生成的每条记录的长度
    std::vector<Condition> fed_conds_;  // 同conds_，两个字段相同

    Rid rid_;
    // std::unique_ptr<RecScan> scan_;  // table_iterator

    SmManager *sm_manager_;
    std::vector<std::unique_ptr<RmRecord>> tuple_buffer;
    std::vector<Rid> rids_buffer;
    int curr_page_in_buffer;
    size_t buffer_idx;
    bool is_end_;

   public:
    // TODO:可优化，针对update，delete之类的可以不需要tuple_buffer
    SeqScanExecutor(SmManager *sm_manager, std::string tab_name, std::vector<Condition> conds, Context *context) {
        // if (context->lock_mgr_->lock_shared_on_table(context_->txn_, fh_->GetFd()) == false) {
        //     // TODO:其他死锁避免方法
        //     // no-wait
        //     throw TransactionAbortException(context_->txn_->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        // }
        sm_manager_ = sm_manager;
        tab_name_ = std::move(tab_name);
        conds_ = std::move(conds);
        sm_manager_->db_.RLatch();
        TabMeta tab = sm_manager_->db_.get_table(tab_name_);
        sm_manager_->db_.RUnLatch();
        tab_ = tab;
        cols_ = tab.cols;
        sm_manager_->latch_.lock_shared();
        fh_ = sm_manager_->fhs_.at(tab_name_).get();
        sm_manager_->latch_.unlock_shared();
        len_ = cols_.back().offset + cols_.back().len;

        context_ = context;
        fed_conds_ = conds_;
        is_end_ = false;
        assert(len_ == fh_->get_record_size());
    }

    void beginTuple() override {

        if (context_->lock_mgr_->lock_shared_on_table(context_->txn_, fh_->GetFd()) == false) {
            // TODO:其他死锁避免方法
            // no-wait
            throw TransactionAbortException(context_->txn_->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        }

        is_end_ = false;
        tuple_buffer.clear();
        rids_buffer.clear();
        buffer_idx = 0;
        bool may_found = false;
        // TODO: 利用 2pl 维护 file_hdr().num_pages
        for (int i = 1; i < fh_->get_file_hdr().num_pages; ++i) {
            RmPageHandle page_handle = fh_->fetch_page_handle(i);
            std::vector<int> slots;
            for (int j = 0; j < fh_->get_file_hdr().num_records_per_page &&
                            slots.size() < static_cast<size_t>(page_handle.page_hdr->num_records);
                 ++j) {
                if (Bitmap::is_set(page_handle.bitmap, j)) {
                    may_found = true;
                    slots.push_back(j);
                }
            }
            if (may_found) {
                for (auto slot_no : slots) {
                    auto rid = Rid{i,slot_no};
                    char *data = page_handle.get_slot(slot_no);
                    auto tmp = std::make_unique<RmRecord>(len_, data);
                    // test conds
                    bool cond_flag = true;
                    for (auto &cond : conds_) {
                        cond_flag = cond_flag && cond.test_record(cols_, tmp);
                        if (!cond_flag) {
                            break;
                        }
                    }
                    if (cond_flag) {
                        tuple_buffer.emplace_back(std::move(tmp));
                        rids_buffer.push_back({i, slot_no});
                    }
                }
            }
            fh_->unpin_page(page_handle.page->get_page_id(), false);
            if (tuple_buffer.size() > 0) {
                curr_page_in_buffer = i;
                buffer_idx = 0;
                return;
            }
        }
        is_end_ = true;
    }

    void nextTuple() override {
        ++buffer_idx;
        if (buffer_idx == tuple_buffer.size()) {
            tuple_buffer.clear();
            rids_buffer.clear();
            for (int i = curr_page_in_buffer + 1; i < fh_->get_file_hdr().num_pages; ++i) {
                bool may_found = false;
                RmPageHandle page_handle = fh_->fetch_page_handle(i);
                std::vector<int> slots;
                for (int j = 0; j < fh_->get_file_hdr().num_records_per_page &&
                                slots.size() < static_cast<size_t>(page_handle.page_hdr->num_records);
                     ++j) {
                    if (Bitmap::is_set(page_handle.bitmap, j)) {
                        may_found = true;
                        slots.push_back(j);
                    }
                }
                if (may_found) {
                    for (auto slot_no : slots) {
                        char *data = page_handle.get_slot(slot_no);
                        auto tmp = std::make_unique<RmRecord>(len_, data);
                        // test conds
                        bool cond_flag = true;
                        for (auto &cond : conds_) {
                            cond_flag = cond_flag && cond.test_record(cols_, tmp);
                            if (!cond_flag) {
                                break;
                            }
                        }
                        if (cond_flag) {
                            tuple_buffer.emplace_back(std::move(tmp));
                            rids_buffer.push_back({i, slot_no});
                        }
                    }
                }
                fh_->unpin_page(page_handle.page->get_page_id(), false);
                if (tuple_buffer.size() > 0) {
                    curr_page_in_buffer = i;
                    buffer_idx = 0;
                    return;
                }
            }
            is_end_ = true;
        }
    }

    bool is_end() const override { return is_end_; }

    std::unique_ptr<RmRecord> Next() override { return std::move(tuple_buffer[buffer_idx]); }

    ColMeta get_col_offset(const TabCol &target) override {
        for (auto &col : cols_) {
            if (col.tab_name == target.tab_name && col.name == target.col_name) {
                return col;
            }
        }
        throw ColumnNotFoundError(target.col_name);
    }
    virtual const std::vector<ColMeta> &cols() const override { return cols_; }
    int tupleLen() const override { return len_; };
    std::string getType() override { return "SeqScanExecutor"; }
    Rid &rid() override { return rids_buffer[buffer_idx]; }
};
