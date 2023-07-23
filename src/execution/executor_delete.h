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

class DeleteExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                   // 表的元数据
    std::vector<Condition> conds_;  // delete的条件
    RmFileHandle *fh_;              // 表的数据文件句柄
    std::vector<Rid> rids_;         // 需要删除的记录的位置
    std::string tab_name_;          // 表名称
    SmManager *sm_manager_;

   public:
    DeleteExecutor(SmManager *sm_manager, const std::string &tab_name,
                   std::vector<Condition> conds, std::vector<Rid> rids,
                   Context *context) {
        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        tab_ = sm_manager_->db_.get_table(tab_name);
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        conds_ = conds;
        rids_ = rids;
        context_ = context;
    }

    std::unique_ptr<RmRecord> Next() override {
        for (auto &rid : rids_) {
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
                fh_->delete_record(rid, context_);
            }
        }

        return nullptr;
    }
    size_t tupleLen() const override { throw UnreachableCodeError(); }

    const std::vector<ColMeta> &cols() const override {
        throw UnreachableCodeError();
    }

    std::string getType() override{ return "DeleteExecutor";}
    // virtual std::string getType() {return "AbstractExecutor";}

    void beginTuple() override{throw  UnreachableCodeError();}

    void nextTuple() override{throw  UnreachableCodeError();}

    bool is_end() const override{throw  UnreachableCodeError();}


    Rid &rid() override { return _abstract_rid; }
};
