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

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include "defs.h"
#include "record/rm_defs.h"
#include "system/sm_meta.h"


struct TabCol {
    std::string tab_name;
    std::string col_name;

    friend bool operator<(const TabCol &x, const TabCol &y) {
        return std::make_pair(x.tab_name, x.col_name) < std::make_pair(y.tab_name, y.col_name);
    }
};

struct Value {
    ColType type;  // type of value
    union {
        int int_val;      // int value
        float float_val;  // float value
    };
    std::string str_val;  // string value

    friend bool operator==(const Value &x, const Value &y) {
        bool ret = false;
        if(x.type==y.type){
            if(x.type == TYPE_INT){
                ret = x.int_val == y.int_val;
            } else if(x.type == TYPE_FLOAT){
                ret  = x.float_val==y.float_val;
            }else  if (x.type == TYPE_STRING){
                ret  =  x.str_val==y.str_val;
            }
        }
        return ret;
        
    }

    friend bool operator!=(const Value &x, const Value &y) { return !(x == y); }
    std::shared_ptr<RmRecord> raw;  // raw record buffer
    static Value read_from_record(std::unique_ptr<RmRecord> &record,ColMeta& col){
        Value ret;
        if(col.type == TYPE_INT){
            int int_val  = *(int*)(record->data+col.offset);
            ret.set_int(int_val);
        } else if(col.type == TYPE_FLOAT){
            float float_val  = *(float*)(record->data+col.offset);
            ret.set_float(float_val);
        }else  if (col.type == TYPE_STRING){
            char* raw_str_val  = record->data+col.offset;
            std::string str_val  = std::string(raw_str_val);
            ret.set_str(str_val);
        }
        return ret;
    }
    void set_int(int int_val_) {
        type = TYPE_INT;
        int_val = int_val_;
    }

    void set_float(float float_val_) {
        type = TYPE_FLOAT;
        float_val = float_val_;
    }

    void set_str(std::string str_val_) {
        type = TYPE_STRING;
        str_val = std::move(str_val_);
    }

    void init_raw(int len) {
        assert(raw == nullptr);
        raw = std::make_shared<RmRecord>(len);
        if (type == TYPE_INT) {
            assert(len == sizeof(int));
            *(int *)(raw->data) = int_val;
        } else if (type == TYPE_FLOAT) {
            assert(len == sizeof(float));
            *(float *)(raw->data) = float_val;
        } else if (type == TYPE_STRING) {
            if (len < (int)str_val.size()) {
                throw StringOverflowError();
            }
            memset(raw->data, 0, len);
            memcpy(raw->data, str_val.c_str(), str_val.size());
        }
    }
};

enum CompOp { OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE };

struct Condition {
    TabCol lhs_col;   // left-hand side column
    CompOp op;        // comparison operator
    bool is_rhs_val;  // true if right-hand side is a value (not a column)
    TabCol rhs_col;   // right-hand side column
    Value rhs_val;    // right-hand side value
    //
    //
    bool test_record(ColMeta& col,std::unique_ptr<RmRecord> &record){
        
        assert(lhs_col.tab_name == col.tab_name);
        

        auto lhs_val= Value::read_from_record(record,col);
        Value real_rhs_val ;
        if(!is_rhs_val){
            auto cond_rhs_col_meta = tab.get_col(rhs_col.col_name);
            real_rhs_val = Value::read_from_record(record,*cond_rhs_col_meta);
        }
        else {
            real_rhs_val = rhs_val;
        }

        return real_rhs_val  == lhs_val;
        
    }
    bool test_join_record(TabMeta & left_tab,std::unique_ptr<RmRecord> &left_rec,TabMeta & right_tab,std::unique_ptr<RmRecord> &right_rec){
        // TODO:  交换? 比如：select * from t1,t2 on t2.id = t1.id;
        // TabCol true_lhs_col,true_rhs_col;
        // if(lhs_col.tab_name == right_tab.name){
        //     true_lhs_col = left_tab.get_col(lhs_co)
        // }

        assert(lhs_col.tab_name ==left_tab.name);
        if(is_rhs_val){
            return test_record(left_tab, left_rec);
        }
        else {
            auto cond_rhs_col_meta = right_tab.get_col(rhs_col.col_name);
            auto real_rhs_val = Value::read_from_record(right_rec,*cond_rhs_col_meta);
            auto cond_lhs_col_meta = left_tab.get_col(lhs_col.col_name);
            auto real_lhs_val = Value::read_from_record(left_rec,*cond_lhs_col_meta);

            return real_lhs_val==real_rhs_val;
        }

    }
};

struct SetClause {
    TabCol lhs;
    Value rhs;
};
