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
#include <cstring>
#include <memory>

#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "record/rm_defs.h"
#include "system/sm.h"

class ProjectionExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> prev_;  // 投影节点的儿子节点
    std::vector<ColMeta> cols_;               // 需要投影的字段
    ssize_t len_;                              // 字段总长度
    std::vector<ssize_t> sel_idxs_;
    RmRecord rec_;

   public:
    ProjectionExecutor(std::unique_ptr<AbstractExecutor> prev,
                       const std::vector<TabCol> &sel_cols) {
        prev_ = std::move(prev);

        ssize_t curr_offset = 0;
        auto &prev_cols = prev_->cols();
        for (auto &sel_col : sel_cols) {
            auto pos = get_col(prev_cols, sel_col);
            sel_idxs_.push_back(pos - prev_cols.begin());
            auto col = *pos;
            col.offset = curr_offset;
            curr_offset += col.len;
            cols_.push_back(col);
        }
        len_ = curr_offset;
        rec_ = RmRecord(len_);
    }

    void beginTuple() override {
        prev_->beginTuple();
        if (!prev_->is_end()) {
            auto record = prev_->Next();

            for (ssize_t i = 0; i < sel_idxs_.size(); i++) {
                auto prev_col = prev_->cols().at(sel_idxs_[i]);
                auto this_col = cols_[i];
                memcpy(rec_.data + this_col.offset,
                       record->data + prev_col.offset, prev_col.len);
            }
        }
    }

    void nextTuple() override {
        prev_->nextTuple();
        if (!prev_->is_end()) {
            auto record = prev_->Next();

            for (ssize_t i = 0; i < sel_idxs_.size(); i++) {
                auto prev_col = prev_->cols().at(sel_idxs_[i]);
                auto this_col = cols_[i];
                memcpy(rec_.data + this_col.offset,
                       record->data + prev_col.offset, prev_col.len);
            }
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        auto ptr = std::make_unique<RmRecord>(len_);
        ptr->SetData(rec_.data);
        return ptr;
    }
    bool is_end ()const  override {
        bool ret = prev_->is_end();
        return ret;
    }

    size_t tupleLen() const override{ return len_; };

    const std::vector<ColMeta> &cols() const override{
        return cols_;
    };

    std::string getType() override{ return "ProjectionExecutor"; };
    Rid &rid() override { return _abstract_rid; }
};
