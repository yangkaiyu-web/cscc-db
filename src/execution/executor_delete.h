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
#include <cassert>
#include <memory>

#include "errors.h"
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class DeleteExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                   // 表的元数据
    std::vector<Condition> conds_;  // delete的条件
    RmFileHandle *fh_;              // 表的数据文件句柄
    std::vector<Rid> rids_;         // 需要删除的记录的位置
    std::string tab_name_;          // 表名称
    SmManager *sm_manager_;

   public:
    DeleteExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<Condition> conds,
                   std::vector<Rid> rids, Context *context) {
        // 如果需要上锁的record多于10000，直接加X锁
        if (!(rids.size() > 10000 &&
              context->lock_mgr_->lock_exclusive_on_table(context->txn_, fh_->GetFd()) == true)) {
            for (auto &rid : rids) {
                if (context->lock_mgr_->lock_exclusive_on_record(context->txn_, rid, fh_->GetFd()) == false) {
                    // TODO:其他死锁避免方法
                    // no-wait
                    throw TransactionAbortException(context_->txn_->get_transaction_id(),
                                                    AbortReason::DEADLOCK_PREVENTION);
                }
            }
        }

        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        sm_manager_->db_.RLatch();
        tab_ = sm_manager_->db_.get_table(tab_name);
        sm_manager_->db_.RUnLatch();
        sm_manager_->latch_.lock_shared();
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        sm_manager_->latch_.unlock_shared();
        conds_ = conds;
        rids_ = rids;
        context_ = context;
    }

    std::unique_ptr<RmRecord> Next() override {
        std::vector<IxIndexHandle *> idx_hdls;
        sm_manager_->latch_.lock_shared();
        for (const IndexMeta &index : tab_.indexes) {
            idx_hdls.push_back(sm_manager_->ihs_.at(index.get_index_name()).get());
        }
        sm_manager_->latch_.unlock_shared();
        auto &indexes = tab_.indexes;

        for (auto &rid : rids_) {
            std::unique_ptr<RmRecord> record = fh_->get_record(rid, context_);
            std::unique_ptr<RmRecord> old_record = fh_->get_record(rid, context_);
            bool cond_flag = true;
            // test conds
            for (auto &cond : conds_) {
                cond_flag = cond_flag && cond.test_record(tab_.cols, record);
                if (!cond_flag) {
                    break;
                }
            }
            if (cond_flag) {
                if(context_->lock_mgr_->lock_exclusive_on_record(context_->txn_, rid, fh_->GetFd())==false){
                    throw TransactionAbortException(context_->txn_->get_transaction_id(),AbortReason::GET_LOCK_FAILED);
                }

                fh_->delete_record(rid, context_);

                for (size_t i = 0; i < indexes.size(); ++i) {
                    char *key = new char[indexes[i].col_tot_len];
                    int offset = 0;
                    for (size_t j = 0; j < static_cast<size_t>(indexes[i].col_num); ++j) {
                        memcpy(key + offset, record->data + indexes[i].cols[j].offset, indexes[i].cols[j].len);
                        offset += indexes[i].cols[j].len;
                    }
                    idx_hdls[i]->delete_entry(key, context_->txn_);
                    delete[] key;
                }
            }
            if (context_->txn_->get_state() == TransactionState::DEFAULT) {
                WriteRecord *delRec = new WriteRecord{WType::DELETE_TUPLE, tab_name_, rid, *old_record};
                context_->txn_->append_write_record(delRec);
            }
        }

        return nullptr;
    }
    int tupleLen() const override { throw UnreachableCodeError(); }

    const std::vector<ColMeta> &cols() const override { throw UnreachableCodeError(); }

    std::string getType() override { return "DeleteExecutor"; }
    // virtual std::string getType() {return "AbstractExecutor";}

    void beginTuple() override { throw UnreachableCodeError(); }

    void nextTuple() override { throw UnreachableCodeError(); }

    bool is_end() const override { throw UnreachableCodeError(); }

    Rid &rid() override { return _abstract_rid; }
};
