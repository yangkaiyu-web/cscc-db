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

#include "errors.h"
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class UpdateExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;
    std::vector<Condition> conds_;
    RmFileHandle *fh_;
    std::vector<Rid> rids_;
    std::string tab_name_;
    std::vector<SetClause> set_clauses_;
    SmManager *sm_manager_;

   public:
    UpdateExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<SetClause> set_clauses,
                   std::vector<Condition> conds, std::vector<Rid> rids, Context *context) {
        // if (!(rids.size() > 10000 &&
        //       context->lock_mgr_->lock_exclusive_on_table(context->txn_, fh_->GetFd()) == true)) {
        //     for (auto &rid : rids) {
        //         if (context->lock_mgr_->lock_exclusive_on_record(context->txn_, rid, fh_->GetFd()) == false) {
        //             // TODO:其他死锁避免方法
        //             // no-wait
        //             throw TransactionAbortException(context_->txn_->get_transaction_id(),
        //                                             AbortReason::DEADLOCK_PREVENTION);
        //         }
        //     }
        // }

        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        set_clauses_ = set_clauses;
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

        std::vector<std::unique_ptr<RmRecord>> old_records, new_records;
        auto &indexes = tab_.indexes;
        for (auto &rid : rids_) {
            if (context_->lock_mgr_->lock_exclusive_on_record(context_->txn_, rid, fh_->GetFd()) == false) {
                throw TransactionAbortException(context_->txn_->get_transaction_id(), AbortReason::GET_LOCK_FAILED);
            }
            auto record = fh_->get_record(rid, context_);

            if (!indexes.empty()) {
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

            old_records.push_back(std::move(record));
        }

        for (int rid_idx = 0; rid_idx < rids_.size(); ++rid_idx) {
            auto new_record = *old_records[rid_idx];

            for (auto &set_clause : set_clauses_) {
                assert(tab_name_ == set_clause.lhs.tab_name);
                auto &col = *tab_.get_col(set_clause.lhs.col_name);
                auto val = set_clause.rhs;
                if (col.type != val.type) {
                    throw IncompatibleTypeError(coltype2str(col.type), coltype2str(val.type));
                }

                if (set_clause.op != '\0') {
                    if (set_clause.op == '+') {
                        if (col.type == ColType::TYPE_INT) {
                            *(int *)(new_record.data + col.offset) += *(int *)val.raw->data;
                        } else if (col.type == ColType::TYPE_FLOAT) {
                            *(float *)(new_record.data + col.offset) += *(float *)val.raw->data;
                        } else if (col.type == ColType::TYPE_BIGINT) {
                            *(size_t *)(new_record.data + col.offset) += *(size_t *)val.raw->data;
                        } else {
                            assert(false);
                        }
                    } else if (set_clause.op == '-') {
                        if (col.type == ColType::TYPE_INT) {
                            *(int *)(new_record.data + col.offset) -= *(int *)val.raw->data;
                        } else if (col.type == ColType::TYPE_FLOAT) {
                            *(float *)(new_record.data + col.offset) -= *(float *)val.raw->data;
                        } else if (col.type == ColType::TYPE_BIGINT) {
                            *(size_t *)(new_record.data + col.offset) -= *(size_t *)val.raw->data;
                        } else {
                            assert(false);
                        }
                    } else {
                        assert(false);
                    }
                } else {
                    memcpy(new_record.data + col.offset, val.raw->data, col.len);
                }
            }

            if (!indexes.empty()) {
                int i = -1;
                char *key;
                try {
                    for (i = 0; i < indexes.size(); ++i) {
                        key = new char[indexes[i].col_tot_len];
                        int offset = 0;
                        for (size_t j = 0; j < static_cast<size_t>(indexes[i].col_num); ++j) {
                            memcpy(key + offset, new_record.data + indexes[i].cols[j].offset, indexes[i].cols[j].len);
                            offset += indexes[i].cols[j].len;
                        }
                        idx_hdls[i]->insert_entry(key, rids_[rid_idx], context_->txn_);
                        delete[] key;
                    }
                } catch (IndexEntryNotUniqueError &e) {
                    delete[] key;
                    int offset = 0;
                    // 先删除
                    for (int delete_index_idx = 0; delete_index_idx < i; ++delete_index_idx) {
                        key = new char[indexes[delete_index_idx].col_tot_len];
                        offset = 0;
                        for (size_t j = 0; j < static_cast<size_t>(indexes[delete_index_idx].col_num); ++j) {
                            memcpy(key + offset, new_record.data + indexes[delete_index_idx].cols[j].offset,
                                   indexes[delete_index_idx].cols[j].len);
                            offset += indexes[delete_index_idx].cols[j].len;
                        }
                        idx_hdls[delete_index_idx]->delete_entry(key, context_->txn_);
                        delete[] key;
                    }

                    for (int delete_rid_idx = 0; delete_rid_idx < rid_idx; ++delete_rid_idx) {
                        for (size_t delete_index_idx = 0; delete_index_idx < indexes.size(); ++delete_index_idx) {
                            key = new char[indexes[delete_index_idx].col_tot_len];
                            offset = 0;
                            for (size_t j = 0; j < static_cast<size_t>(indexes[delete_index_idx].col_num); ++j) {
                                memcpy(key + offset,
                                       new_records[delete_rid_idx]->data + indexes[delete_index_idx].cols[j].offset,
                                       indexes[delete_index_idx].cols[j].len);
                                offset += indexes[delete_index_idx].cols[j].len;
                            }
                            idx_hdls[delete_index_idx]->delete_entry(key, context_->txn_);
                            delete[] key;
                        }
                    }

                    // 插入
                    for (int insert_rid_idx = 0; insert_rid_idx < rids_.size(); ++insert_rid_idx) {
                        for (size_t insert_index_idx = 0; insert_index_idx < indexes.size(); ++insert_index_idx) {
                            offset = 0;
                            key = new char[indexes[insert_index_idx].col_tot_len];
                            for (size_t j = 0; j < static_cast<size_t>(indexes[insert_index_idx].col_num); ++j) {
                                memcpy(key + offset,
                                       old_records[insert_rid_idx]->data + indexes[insert_index_idx].cols[j].offset,
                                       indexes[insert_index_idx].cols[j].len);
                                offset += indexes[insert_index_idx].cols[j].len;
                            }
                            idx_hdls[insert_index_idx]->insert_entry(key, rids_[insert_rid_idx], context_->txn_);
                            delete[] key;
                        }
                    }
                    throw e;
                }
            }
            new_records.push_back(std::make_unique<RmRecord>(std::move(new_record)));
        }

        for (size_t rid_idx = 0; rid_idx < rids_.size(); ++rid_idx) {
            RmRecord record = *new_records[rid_idx];
            // 更新事务
            if (context_->txn_->get_state() == TransactionState::DEFAULT ||
                context_->txn_->get_state() == TransactionState::GROWING) {
                auto WriteRec =
                    std::make_unique<WriteRecord>(WType::UPDATE_TUPLE, tab_name_, rids_[rid_idx],
                                                  std::move(*old_records[rid_idx]), std::move(*new_records[rid_idx]));
                context_->log_mgr_->gen_log_from_write_set(context_->txn_, WriteRec.get());
                context_->txn_->append_write_record(std::move(WriteRec));
            } else {
                assert(false);
            }
            fh_->update_record(rids_[rid_idx], record.data, context_);
        }

        return nullptr;
    }

    int tupleLen() const override { throw UnreachableCodeError(); }

    const std::vector<ColMeta> &cols() const override { throw UnreachableCodeError(); }

    std::string getType() override { return "UpdateExecutor"; }

    void beginTuple() override { throw UnreachableCodeError(); }

    void nextTuple() override { throw UnreachableCodeError(); }

    bool is_end() const override { throw UnreachableCodeError(); }

    Rid &rid() override { return _abstract_rid; }
};
