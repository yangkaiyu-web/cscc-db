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
#include <cstring>
#include <string>
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "common/config.h"
#include "index/ix.h"
#include "system/sm.h"
#include "system/sm_manager.h"

class SortExecutor : public AbstractExecutor {

   private:
    // TODO: 直接申请内存还是从bufferpool里获取？

    static const int mem_num = 10;  // 花费 10 页内存用于排序  5 页用于缓存输入， 5页用于缓存输出
    SmManager *sm_manager_;
    std::unique_ptr<AbstractExecutor> prev_;
    std::vector<ColMeta> cols_;                              // 框架中只支持一个键排序，需要自行修改数据结构支持多个键排序
    size_t tuple_num;
    int limit_num_;
    bool is_desc_;
    std::vector<size_t> used_tuple;
    std::unique_ptr<RmRecord> current_tuple;

   public:
    SortExecutor(SmManager* sm_manager,std::unique_ptr<AbstractExecutor> prev, std::vector<TabCol> sel_cols, bool is_desc,int limit_num) {
        prev_ = std::move(prev);
        for(auto& col : sel_cols){
            cols_.push_back(prev_->get_col_offset(col));
        }
        is_desc_ = is_desc;
        tuple_num = 0;
        limit_num_ = limit_num;
        used_tuple.clear();
    }
// TODO: 缓存一下排序结果，防止反复 begin
    void beginTuple() override { 
        std::vector<std::string> temp_file_name;
        int temp_file_count = 0;
        auto dm = sm_manager_->get_disk_manager();
        auto tuple_num_per_page  =  PAGE_SIZE /prev_->tupleLen();
        auto tuple_len = prev_->tupleLen();
        prev_->beginTuple();
        int tuple_num_on_page=0;
        while(!prev_->is_end()){
            char read_buf[PAGE_SIZE];
            auto tuple =prev_->Next();
            memcpy(read_buf+tuple_len*tuple_num_on_page, tuple->data, tuple_len);
            if(tuple_num_on_page == tuple_num_per_page){

            }
        }


    }

    void nextTuple() override {
        
    }

    std::unique_ptr<RmRecord> Next() override {
        return nullptr;
    }

    Rid &rid() override { return _abstract_rid; }
};
