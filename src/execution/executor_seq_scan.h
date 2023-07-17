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

class SeqScanExecutor : public AbstractExecutor {
   private:
    std::string tab_name_;  // 表的名称
    TabMeta tab_;
    std::vector<Condition> conds_;      // scan的条件
    RmFileHandle *fh_;                  // 表的数据文件句柄
    std::vector<ColMeta> cols_;         // scan后生成的记录的字段
    size_t len_;                        // scan后生成的每条记录的长度
    std::vector<Condition> fed_conds_;  // 同conds_，两个字段相同

    Rid rid_;
    std::unique_ptr<RecScan> scan_;  // table_iterator

    SmManager *sm_manager_;

   public:
    SeqScanExecutor(SmManager *sm_manager, std::string tab_name,
                    std::vector<Condition> conds, Context *context) {
        sm_manager_ = sm_manager;
        tab_name_ = std::move(tab_name);
        conds_ = std::move(conds);
        TabMeta &tab = sm_manager_->db_.get_table(tab_name_);
        tab_ = tab;
        fh_ = sm_manager_->fhs_.at(tab_name_).get();
        cols_ = tab.cols;
        len_ = cols_.back().offset + cols_.back().len;

        context_ = context;

        fed_conds_ = conds_;
    }

    void beginTuple() override {
        scan_ = std::make_unique<RmScan>(fh_);
        for (auto rid = scan_->rid(); !scan_->is_end(); scan_->next()) {
            rid = scan_->rid();
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
                rid_ = rid;
                return ;
            }
        }
    }

    void nextTuple() override {
        Rid rid;
        scan_->next();
        while(!scan_->is_end()){
            rid = scan_->rid();
            
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
                rid_ = rid;
                return ;
            }
            
        scan_->next();
        }
    }
    bool is_end() const override {
        bool ret = scan_->is_end();
        return ret;
    }

    std::unique_ptr<RmRecord> Next() override { return fh_->get_record(rid_, context_); }

    virtual const std::vector<ColMeta> &cols() const {
        return cols_;
    }
    size_t tupleLen() const { return len_; };
    std::string getType()override{return "SeqScanExecutor";}
    Rid &rid() override { return rid_; }
};

