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

#include "errors.h"
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "record/rm_defs.h"
#include "system/sm.h"

class NestedLoopJoinExecutor : public AbstractExecutor {
   private:
    // left_record_save_最大存储元组数
    static const size_t OUTER_BUFFER_SIZE = 4000;
    // right_record_save_最大存储元组数
    static const size_t INNER_BUFFER_SIZE = 2000;
    std::unique_ptr<AbstractExecutor> left_;   // 左儿子节点（需要join的表）
    std::unique_ptr<AbstractExecutor> right_;  // 右儿子节点（需要join的表）
    ssize_t len_;                              // join后获得的每条记录的长度
    const std::vector<ColMeta>& left_cols_;    // join前的字段
    const std::vector<ColMeta>& right_cols_;   // join前的字段
    std::vector<ColMeta> cols_;                // join后获得的记录的字段

    std::vector<Condition> fed_conds_;         // join条件
    bool is_end_;
    std::vector<std::unique_ptr<RmRecord>> left_record_save_;
    size_t left_index_;
    std::vector<std::unique_ptr<RmRecord>> right_record_save_;
    size_t right_index_;

   public:
    NestedLoopJoinExecutor(std::unique_ptr<AbstractExecutor> left, std::unique_ptr<AbstractExecutor> right,
                           std::vector<Condition> conds)
        : left_(std::move(left)),
          right_(std::move(right)),
          len_(left_->tupleLen() + right_->tupleLen()),
          left_cols_(left_->cols()),
          right_cols_(right_->cols()) {
        cols_ = left_cols_;
        cols_.insert(cols_.end(), right_cols_.begin(), right_cols_.end());

        for (size_t i = left_cols_.size(); i < cols_.size(); ++i) {
            cols_[i].offset += left_->tupleLen();
        }

        left_record_save_.reserve(OUTER_BUFFER_SIZE);
        right_record_save_.reserve(INNER_BUFFER_SIZE);
        left_index_ = 0;
        right_index_ = 0;
        is_end_ = false;
        fed_conds_ = std::move(conds);
        // left_index_ = 0;  // 标志当前已经消耗完第几个
    }

    void beginTuple() override {
        // TODO:  交换? 比如：select * from t1,t2 on t2.id = t1.id;
        left_record_save_.clear();
        left_->beginTuple();
        while (left_record_save_.size() < OUTER_BUFFER_SIZE && !left_->is_end()) {
            left_record_save_.emplace_back(left_->Next());
            left_->nextTuple();
        }
        left_index_ = 0;
        if (left_record_save_.size() == 0) {
            assert(left_->is_end());
            is_end_ = true;
            return;
        }

        right_record_save_.clear();
        right_->beginTuple();
        while (right_record_save_.size() < INNER_BUFFER_SIZE && !right_->is_end()) {
            right_record_save_.emplace_back(right_->Next());
            right_->nextTuple();
        }
        if (right_record_save_.size() == 0) {
            assert(right_->is_end());
            is_end_ = true;
            return;
        }
        right_index_ = 0;

        while (true) {
            for (; left_index_ < left_record_save_.size(); ++left_index_) {
                for (; right_index_ < right_record_save_.size(); ++right_index_) {
                    bool flag = true;
                    for (auto& cond : fed_conds_) {
                        flag = flag && cond.test_join_record(left_cols_, left_record_save_[left_index_], right_cols_,
                                                             right_record_save_[right_index_]);
                        if (flag == false) break;
                    }
                    if (flag) {
                        return;
                    }
                }
                right_index_ = 0;
            }
            left_index_ = 0;

            right_record_save_.clear();
            while (right_record_save_.size() < INNER_BUFFER_SIZE && !right_->is_end()) {
                right_record_save_.emplace_back(right_->Next());
                right_->nextTuple();
            }
            if (right_record_save_.size() == 0) {
                // inner已经被全部遍历，outer需要装入下一个块
                left_record_save_.clear();
                while (left_record_save_.size() < OUTER_BUFFER_SIZE && !left_->is_end()) {
                    left_record_save_.emplace_back(left_->Next());
                    left_->nextTuple();
                }
                if (left_record_save_.size() == 0) {
                    assert(left_->is_end());
                    is_end_ = true;
                    return;
                }
                left_index_ = 0;

                // inner需要从头开始
                right_->beginTuple();
                while (right_record_save_.size() < INNER_BUFFER_SIZE && !right_->is_end()) {
                    right_record_save_.emplace_back(right_->Next());
                    right_->nextTuple();
                }
            }
            // right_index_ = 0;
        }
    }

    void nextTuple() override {
        ++right_index_;
        while (true) {
            for (; left_index_ < left_record_save_.size(); ++left_index_) {
                for (; right_index_ < right_record_save_.size(); ++right_index_) {
                    bool flag = true;
                    for (auto& cond : fed_conds_) {
                        flag = flag && cond.test_join_record(left_cols_, left_record_save_[left_index_], right_cols_,
                                                             right_record_save_[right_index_]);
                        if (flag == false) break;
                    }
                    if (flag) {
                        return;
                    }
                }
                right_index_ = 0;
            }
            left_index_ = 0;

            right_record_save_.clear();
            while (right_record_save_.size() < INNER_BUFFER_SIZE && !right_->is_end()) {
                right_record_save_.emplace_back(right_->Next());
                right_->nextTuple();
            }
            if (right_record_save_.size() == 0) {
                assert(right_->is_end());
                // inner已经被全部遍历，outer需要装入下一个块
                left_record_save_.clear();
                while (left_record_save_.size() < OUTER_BUFFER_SIZE && !left_->is_end()) {
                    left_record_save_.emplace_back(left_->Next());
                    left_->nextTuple();
                }
                if (left_record_save_.size() == 0) {
                    assert(left_->is_end());
                    is_end_ = true;
                    return;
                }
                left_index_ = 0;

                // inner需要从头开始
                right_->beginTuple();
                while (right_record_save_.size() < INNER_BUFFER_SIZE && !right_->is_end()) {
                    right_record_save_.emplace_back(right_->Next());
                    right_->nextTuple();
                }
            }
            // right_index_ = 0;
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        auto rec = std::make_unique<RmRecord>(len_);
        memcpy(rec->data, left_record_save_[left_index_]->data, left_->tupleLen());
        memcpy(rec->data + left_->tupleLen(), right_record_save_[right_index_]->data, right_->tupleLen());
        return rec;
    }

    bool is_end() const override { return is_end_; }

    int tupleLen() const override { return len_; };

    const std::vector<ColMeta>& cols() const override { return cols_; };

    ColMeta get_col_offset(const TabCol& target) override {
        for (auto& col : cols_) {
            if (col.tab_name == target.tab_name && col.name == target.col_name) {
                return col;
            }
        }
        throw ColumnNotFoundError(target.col_name);
    }
    std::string getType() override { return "NestedLoopJoinExecutor"; };
    Rid& rid() override { return _abstract_rid; }

   private:
    bool outer_is_end_ = false;
    // 负责维护left_index
    void outer_begin() {
        left_record_save_.clear();
        left_->beginTuple();
        while (left_record_save_.size() < OUTER_BUFFER_SIZE && !left_->is_end()) {
            left_record_save_.emplace_back(left_->Next());
            left_->nextTuple();
        }
        left_index_ = 0;
        if (left_record_save_.size() == 0) {
            assert(left_->is_end());
            outer_is_end_ = true;
        }
    }
    // 负责维护left_index
    void outer_next() {
        left_index_++;
        if (left_index_ == left_record_save_.size()) {
            if (left_->is_end()) {
                outer_is_end_ = true;
                return;
            }
            left_record_save_.clear();
            while (left_record_save_.size() < OUTER_BUFFER_SIZE && !left_->is_end()) {
                left_record_save_.emplace_back(left_->Next());
                left_->nextTuple();
            }
            left_index_ = 0;
        }
    }

    bool outer_is_end() { return outer_is_end_; }

    bool inner_is_end_ = false;
    // 负责维护right_index
    void inner_begin() {
        inner_is_end_ = false;
        right_record_save_.clear();
        right_->beginTuple();
        while (right_record_save_.size() < INNER_BUFFER_SIZE && !right_->is_end()) {
            auto res = right_->Next();
            assert(res->allocated_ == true);
            right_record_save_.emplace_back(std::move(res));
            right_->nextTuple();
        }
        right_index_ = 0;
        if (right_record_save_.size() == 0) {
            assert(right_->is_end());
            inner_is_end_ = true;
        }
    }
    // 负责维护right_index
    void inner_next() {
        right_index_++;
        if (right_index_ == right_record_save_.size()) {
            if (right_->is_end()) {
                inner_is_end_ = true;
                return;
            }
            right_record_save_.clear();
            while (right_record_save_.size() < INNER_BUFFER_SIZE && !right_->is_end()) {
                right_record_save_.emplace_back(right_->Next());
                right_->nextTuple();
            }
            right_index_ = 0;
        }
    }

    bool inner_is_end() { return inner_is_end_; }
};
