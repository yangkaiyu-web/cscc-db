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

class InsertExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                // 表的元数据
    std::vector<Value> values_;  // 需要插入的数据
    RmFileHandle *fh_;           // 表的数据文件句柄
    std::string tab_name_;       // 表名称
    Rid rid_;  // 插入的位置，由于系统默认插入时不指定位置，因此当前rid_在插入后才赋值
    SmManager *sm_manager_;

   public:
    InsertExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<Value> values, Context *context) {
        sm_manager_ = sm_manager;
        sm_manager_->db_.RLatch();
        tab_ = sm_manager_->db_.get_table(tab_name);
        sm_manager_->db_.RUnLatch();
        values_ = values;
        tab_name_ = tab_name;
        if (values.size() != tab_.cols.size()) {
            throw InvalidValueCountError();
        }
        sm_manager_->latch_.lock_shared();
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        sm_manager_->latch_.unlock_shared();
        context_ = context;
    };

    std::unique_ptr<RmRecord> Next() override {
        // Make record buffer
        RmRecord rec(fh_->get_file_hdr().record_size);
        for (size_t i = 0; i < values_.size(); i++) {
            auto &col = tab_.cols[i];
            auto &val = values_[i];
            if (col.type != val.type) {
                throw IncompatibleTypeError(coltype2str(col.type), coltype2str(val.type));
            }
            memcpy(rec.data + col.offset, val.raw->data, col.len);
        }

        std::vector<IxIndexHandle *> idx_hdls;
        sm_manager_->latch_.lock_shared();
        for (const IndexMeta &index : tab_.indexes) {
            idx_hdls.push_back(sm_manager_->ihs_.at(index.get_index_name()).get());
        }
        sm_manager_->latch_.unlock_shared();

        // 检查以符合索引唯一性
        for (size_t i = 0; i < tab_.indexes.size(); ++i) {
            auto &index = tab_.indexes[i];
            std::unique_ptr<char[]> key(new char[index.col_tot_len], std::default_delete<char[]>());
            int offset = 0;
            assert(index.col_num >= 0);
            for (size_t j = 0; j < static_cast<size_t>(index.col_num); ++j) {
                memcpy(key.get() + offset, rec.data + index.cols[j].offset, index.cols[j].len);
                offset += index.cols[j].len;
            }
            if (idx_hdls[i]->is_exist(key.get(), context_->txn_)) {
                throw IndexEntryNotUniqueError();
            }
        }

        if (tab_.indexes.size() == 0) {
            if (context_->lock_mgr_->lock_exclusive_on_record(context_->txn_, rid_, fh_->GetFd()) == false) {
                throw TransactionAbortException(context_->txn_->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
            }
        } else {
            // Insert into index file
            for (size_t i = 0; i < tab_.indexes.size(); ++i) {
                auto &index = tab_.indexes[i];
                std::unique_ptr<char[]> key(new char[index.col_tot_len], std::default_delete<char[]>());
                int offset = 0;
                assert(index.col_num >= 0);
                for (size_t j = 0; j < static_cast<size_t>(index.col_num); ++j) {
                    memcpy(key.get() + offset, rec.data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                idx_hdls[i]->insert_entry(key.get(), rid_, context_->txn_);
                if (context_->txn_->get_state() == TransactionState::DEFAULT) {
                    WriteIndex *ins_rec = new WriteIndex(WType::INSERT_INDEX, idx_hdls[i], rid_, std::move(key));
                    context_->txn_->append_write_index(ins_rec);
                }
            }
        }

        // Insert into record file
        rid_ = fh_->insert_record(rec.data, context_);

        if (context_->txn_->get_state() == TransactionState::DEFAULT) {
            WriteRecord *ins_rec = new WriteRecord(WType::INSERT_TUPLE, fh_, rid_);
            context_->txn_->append_write_record(ins_rec);
        }

        return nullptr;
    }

    Rid &rid() override { return rid_; }

    void beginTuple() override { throw UnreachableCodeError(); }

    void nextTuple() override { throw UnreachableCodeError(); }

    bool is_end() const override { throw UnreachableCodeError(); }
    int tupleLen() const override { throw UnreachableCodeError(); }
    std::string getType() override { return "InsertExecutor"; }

    const std::vector<ColMeta> &cols() const override { throw UnreachableCodeError(); }
};
