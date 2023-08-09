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
        auto &indexes = tab_.indexes;
        // 检查索引唯一性
        for (auto &rid : rids_) {
            auto record = fh_->get_record(rid, context_);
            for (auto &set_clause : set_clauses_) {
                assert(tab_name_ == set_clause.lhs.tab_name);
                auto &col = *tab_.get_col(set_clause.lhs.col_name);
                auto val = set_clause.rhs;
                if (col.type != val.type) {
                    throw IncompatibleTypeError(coltype2str(col.type), coltype2str(val.type));
                }
                memcpy(record->data + col.offset, val.raw->data, col.len);
            }

            for (size_t i = 0; i < indexes.size(); ++i) {
                char *key = new char[indexes[i].col_tot_len];
                int offset = 0;
                for (size_t j = 0; j < static_cast<size_t>(indexes[i].col_num); ++j) {
                    memcpy(key + offset, record->data + indexes[i].cols[j].offset, indexes[i].cols[j].len);
                    offset += indexes[i].cols[j].len;
                }

                std::vector<Rid> result;
                if (idx_hdls[i]->get_value(key, result, nullptr) && result[0] != rid) {
                    assert(result.size() == 1);
                    delete[] key;
                    throw IndexEntryNotUniqueError();
                }
                delete[] key;
            }
        }

        for (auto &rid : rids_) {
            auto record = fh_->get_record(rid, context_);
            auto old_record = fh_->get_record(rid, context_);
            char *old_raw_data = new char[record->size];
            memcpy(old_raw_data, record->data, record->size);

            for (auto &set_clause : set_clauses_) {
                assert(tab_name_ == set_clause.lhs.tab_name);
                auto &col = *tab_.get_col(set_clause.lhs.col_name);
                auto val = set_clause.rhs;
                if (col.type != val.type) {
                    throw IncompatibleTypeError(coltype2str(col.type), coltype2str(val.type));
                }
                memcpy(record->data + col.offset, val.raw->data, col.len);
            }
            fh_->update_record(rid, record->data, context_);
            // 更新事务
            if (context_->txn_->get_state() == TransactionState::DEFAULT) {
                auto WriteRec = new WriteRecord(WType::UPDATE_TUPLE, tab_name_, rid, *old_record);
                context_->txn_->append_write_record(WriteRec);
            }

            // 更新索引项

            for (size_t i = 0; i < indexes.size(); ++i) {
                char *key = new char[indexes[i].col_tot_len];
                int offset = 0;
                for (size_t j = 0; j < static_cast<size_t>(indexes[i].col_num); ++j) {
                    memcpy(key + offset, old_raw_data + indexes[i].cols[j].offset, indexes[i].cols[j].len);
                    offset += indexes[i].cols[j].len;
                }
                idx_hdls[i]->delete_entry(key, context_->txn_);

                offset = 0;
                for (size_t j = 0; j < static_cast<size_t>(indexes[i].col_num); ++j) {
                    memcpy(key + offset, record->data + indexes[i].cols[j].offset, indexes[i].cols[j].len);
                    offset += indexes[i].cols[j].len;
                }
                idx_hdls[i]->insert_entry(key, rid, context_->txn_);
            }
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
