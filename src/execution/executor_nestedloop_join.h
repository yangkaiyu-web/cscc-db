/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
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
#include "record/rm_defs.h"
#include "system/sm.h"

class NestedLoopJoinExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> left_;    // 左儿子节点（需要join的表）
    std::unique_ptr<AbstractExecutor> right_;   // 右儿子节点（需要join的表）
    ssize_t len_;                                // join后获得的每条记录的长度
    std::vector<ColMeta> cols_;                 // join后获得的记录的字段

    std::vector<Condition> fed_conds_;          // join条件
    bool is_end_;
    RmRecord rec_;
  std::vector<std::unique_ptr<RmRecord>> left_record_save_;

   public:
    NestedLoopJoinExecutor(std::unique_ptr<AbstractExecutor> left, std::unique_ptr<AbstractExecutor> right, 
                            std::vector<Condition> conds) {
        left_ = std::move(left);
        right_ = std::move(right);
        len_ = left_->tupleLen() + right_->tupleLen();
        cols_ = left_->cols();
        auto right_cols = right_->cols();
        for (auto &col : right_cols) {
            col.offset += left_->tupleLen();
        }

        cols_.insert(cols_.end(), right_cols.begin(), right_cols.end());
        is_end_ = false;
        fed_conds_ = std::move(conds);

    }

    void beginTuple() override {
        // TODO:  交换? 比如：select * from t1,t2 on t2.id = t1.id;
        for(left_->beginTuple();!left_->is_end();left_->nextTuple()){
          left_record_save_.push_back(left_->Next());
        }
        if(!left_record_save_.empty()){
            for(right_->beginTuple();!right_->is_end();right_->nextTuple()){

                auto right_rec =  right_->Next();
                for(auto& left_record:left_record_save_){

                    auto flag = true;
                    for(auto& cond: fed_conds_){

                        flag = flag && cond.test_join_record(left_->cols(),left_record,right_->cols(),right_rec);
                    }
                    if(flag){
                        memcpy(rec_.data,left_record->data,left_->tupleLen());
                        memcpy(rec_.data+left_->tupleLen(),right_rec->data,right_->tupleLen());
                        return;
                    }
                }
            }
        }


    }

    void nextTuple() override {
        right_->nextTuple();
        while(!right_->is_end()){
                auto right_rec =  right_->Next();
                for(auto& left_record:left_record_save_){

                    auto flag = true;
                    for(auto& cond: fed_conds_){

                        flag = flag && cond.test_join_record(left_->cols(),left_record,right_->cols(),right_rec);
                    }
                    if(flag){
                        memcpy(rec_.data,left_record->data,left_->tupleLen());
                        memcpy(rec_.data+left_->tupleLen(),right_rec->data,right_->tupleLen());
                        return;
                    }
                }
            right_->nextTuple();
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        auto ptr = std::make_unique<RmRecord>(len_);
        ptr->SetData(rec_.data);
        return ptr;
    }
    
    bool is_end ()const  override {
        bool ret = right_->is_end() || left_record_save_.empty();
        return ret;
    }

    Rid &rid() override { return _abstract_rid; }
};
