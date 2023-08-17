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
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "analyze/analyze.h"
#include "defs.h"
#include "errors.h"
#include "parser/ast.h"
#include "record/rm_defs.h"
#include "system/sm_meta.h"

enum AggType { COUNT, MAX, MIN, SUM };

struct TabCol {
    std::string tab_name;
    std::string col_name;
    bool has_agg;
    AggType aggregate_type;
    std::string another_name = "";
    std::string agg_arg_col_name;

    friend bool operator<(const TabCol &x, const TabCol &y) {
        return std::make_pair(x.tab_name, x.col_name) < std::make_pair(y.tab_name, y.col_name);
    }
};

struct Value {
    ColType type;  // type of value
    union {
        int int_val;         // int value
        int64_t bigint_val;  // bigint or datatime value,
                             // datatime时，40~56位年，32~40位月，24~32位日，16~24位时，8~16位分，0~8位秒
        float float_val;  // float value
    };
    std::string str_val;            // string value
    std::shared_ptr<RmRecord> raw;  // raw record buffer

    friend bool operator==(const Value &x, const Value &y) {
        bool ret = false;
        if (x.type == y.type) {
            if (x.type == TYPE_INT) {
                ret = x.int_val == y.int_val;
            } else if (x.type == TYPE_FLOAT) {
                ret = x.float_val == y.float_val;
            } else if (x.type == TYPE_STRING) {
                ret = x.str_val == y.str_val;
            } else if (x.type == TYPE_BIGINT || x.type == TYPE_DATETIME) {
                ret = x.bigint_val == y.bigint_val;
            }
        }
        return ret;
    }

    friend bool operator!=(const Value &x, const Value &y) { return !(x == y); }
    friend bool operator>(const Value &x, const Value &y) {
        bool ret = false;
        if (x.type == y.type) {
            if (x.type == TYPE_INT) {
                ret = x.int_val > y.int_val;
            } else if (x.type == TYPE_BIGINT) {
                ret = x.bigint_val > y.bigint_val;
            } else if (x.type == TYPE_FLOAT) {
                ret = x.float_val > y.float_val;
            } else if (x.type == TYPE_STRING) {
                ret = x.str_val > y.str_val;
            } else if (x.type == TYPE_DATETIME) {
                ret = static_cast<uint64_t>(x.bigint_val) > static_cast<uint64_t>(y.bigint_val);
            }
        }
        return ret;
    }
    friend bool operator<(const Value &x, const Value &y) {
        bool ret = false;
        if (x.type == y.type) {
            if (x.type == TYPE_INT) {
                ret = x.int_val < y.int_val;
            } else if (x.type == TYPE_BIGINT) {
                ret = x.bigint_val < y.bigint_val;
            } else if (x.type == TYPE_FLOAT) {
                ret = x.float_val < y.float_val;
            } else if (x.type == TYPE_STRING) {
                ret = x.str_val < y.str_val;
            } else if (x.type == TYPE_DATETIME) {
                ret = static_cast<uint64_t>(x.bigint_val) < static_cast<uint64_t>(y.bigint_val);
            }
        }
        return ret;
    }
    // MyNumber operator+(const MyNumber& other) const {
    //        MyNumber result(value + other.value);
    //        return result;
    //    }
    friend Value operator+(const Value &x, const Value &y) {
        Value ret;
        if (x.type == TYPE_FLOAT && y.type == TYPE_FLOAT) {
            float tmp = x.float_val + y.float_val;
            ret.set_float(tmp);
        } else if (x.type == TYPE_INT && y.type == TYPE_INT) {
            int tmp = x.int_val + y.int_val;
            ret.set_int(tmp);
        } else {
            throw InternalError("error value plus operator type");
        }
        return ret;
    }
    friend bool operator<=(const Value &x, const Value &y) { return !(x > y); }
    friend bool operator>=(const Value &x, const Value &y) { return !(x < y); }
    static Value read_from_record(const std::unique_ptr<RmRecord> &record, ColMeta &col) {
        Value ret;
        if (col.type == TYPE_INT) {
            int int_val = *(int *)(record->data + col.offset);
            ret.set_int(int_val);
        } else if (col.type == TYPE_BIGINT) {
            int64_t bigint_val = *(int64_t *)(record->data + col.offset);
            ret.set_bigint(bigint_val);
        } else if (col.type == TYPE_FLOAT) {
            float float_val = *(float *)(record->data + col.offset);
            ret.set_float(float_val);
        } else if (col.type == TYPE_STRING) {
            char *raw_str_val = record->data + col.offset;
            int str_len = (static_cast<int>(strlen(raw_str_val)) > col.len) ? col.len : strlen(raw_str_val);

            std::string str_val = std::string(raw_str_val, str_len);
            ret.set_str(str_val);
        } else if (col.type == TYPE_DATETIME) {
            int64_t bigint_val = *(int64_t *)(record->data + col.offset);
            ret.set_datetime(bigint_val);
        }
        return ret;
    }

    static Value convert_from_string(const std::string &str, ColMeta &col) {
        Value val;
        // if (auto int_bint_lit = std::dynamic_pointer_cast<ast::Int_Bint_Lit>(sv_val)) {
        //     const std::string &int_bint = int_bint_lit->val;
        if (col.type == TYPE_BIGINT) {
            if (no_overflow<int64_t>(str)) {
                val.set_bigint(std::stoll(str));
            } else {
                throw BigIntOverflowError(str);
            }
        } else if (col.type == TYPE_INT) {
            // 检查范围防止溢出
            if (no_overflow<int>(str)) {
                val.set_int(static_cast<int>(std::stoi(str)));
            } else {
                throw IntOverflowError(str);
            }
        } else if (col.type == TYPE_FLOAT) {
            if (no_overflow<float>(str)) {
                val.set_float((static_cast<float>(std::stof(str))));
            } else {
                throw FloatOverflowError(str);
            }
        } else if (col.type == TYPE_STRING) {
            val.set_str(str);
        } else if (col.type == TYPE_DATETIME) {
            val.check_set_datetime(str);
        } else {
            throw InternalError("Unexpected sv value type");
        }

        // } else if (auto float_lit = std::dynamic_pointer_cast<ast::FloatLit>(sv_val)) {
        //     if (col->type == TYPE_FLOAT) {
        //         val.set_float(float_lit->val);
        //     } else if (col->type == TYPE_INT) {
        //         val.set_int(static_cast<int>(float_lit->val));
        //     } else if (col->type == TYPE_BIGINT) {
        //         val.set_bigint(static_cast<int64_t>(float_lit->val));
        //     } else {
        //         throw CastTypeError("float", coltype2str(col->type));
        //     }
        // } else if (auto str_lit = std::dynamic_pointer_cast<ast::StringLit>(sv_val)) {
        //     if (col->type == TYPE_STRING) {
        //         val.set_str(str_lit->val);
        //     } else if (col->type == TYPE_DATETIME) {
        //         val.check_set_datetime(str_lit->val);
        //     } else {
        //         throw CastTypeError("string", coltype2str(col->type));
        //     }
        // } else {
        //     throw InternalError("Unexpected sv value type");
        // }
        val.init_raw(col.len);
        return val;
    }
    void set_int(int int_val_) {
        type = TYPE_INT;
        int_val = int_val_;
    }
    void set_bigint(int64_t bigint_val_) {
        type = TYPE_BIGINT;
        bigint_val = bigint_val_;
    }

    /*
    void cast_to(ColType type_to) {
        if (type_to == TYPE_STRING || type == TYPE_STRING) {
            throw CastTypeError(coltype2str(type), coltype2str(type_to));
        }
        if (type_to == TYPE_INT && type == TYPE_FLOAT) {
            set_int((int)float_val);
        } else if (type_to == TYPE_FLOAT && type == TYPE_INT) {
            set_float((float)int_val);
        }
    }*/

    void set_float(float float_val_) {
        type = TYPE_FLOAT;
        float_val = float_val_;
    }

    void set_str(std::string str_val_) {
        type = TYPE_STRING;
        str_val = std::move(str_val_);
    }

    void set_datetime(int64_t date_val_) {
        type = TYPE_DATETIME;
        bigint_val = date_val_;
    }

    bool IsDayValid(int year, int month, int day) {
        if (month == 4 || month == 6 || month == 9 || month == 11) {
            return day <= 30;
        }

        if (month == 2) {
            if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
                return day <= 29;
            else
                return day <= 28;
        }

        if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
            return day <= 31;

        return false;
    }

    // 格式为YYYY-MM-DD HH:MM:SS
    void check_set_datetime(const std::string &str_val_) {
        if (str_val_.size() == 19 && str_val_[4] == '-' && str_val_[7] == '-' && str_val_[10] == ' ' &&
            str_val_[13] == ':' && str_val_[16] == ':') {
            const std::string &syear = str_val_.substr(0, 4);
            const std::string &smonth = str_val_.substr(5, 2);
            const std::string &sday = str_val_.substr(8, 2);
            const std::string &shour = str_val_.substr(11, 2);
            const std::string &sminute = str_val_.substr(14, 2);
            const std::string &ssec = str_val_.substr(17, 2);

            int year = std::stoi(syear);
            int month = std::stoi(smonth);
            int day = std::stoi(sday);

            if (syear >= "1000" && syear <= "9999" && smonth >= "01" && smonth <= "12" && sday >= "01" &&
                sday <= "31" && shour >= "00" && shour <= "23" && sminute >= "00" && sminute <= "59" && ssec >= "00" &&
                ssec <= "59" && IsDayValid(year, month, day)) {
                bigint_val = (static_cast<int64_t>(year) << 40) + (static_cast<int64_t>(month) << 32) +
                             (static_cast<int64_t>(day) << 24) + (std::stoll(shour) << 16) +
                             (std::stoll(sminute) << 8) + (std::stoll(ssec));
                type = TYPE_DATETIME;
                return;
            }
        }

        throw DateTimeFormatError(str_val_);
    }

    void init_raw(int len) {
        assert(raw == nullptr);
        raw = std::make_shared<RmRecord>(len);
        if (type == TYPE_INT) {
            assert(len == sizeof(int));
            *(int *)(raw->data) = int_val;
        } else if (type == TYPE_BIGINT || type == TYPE_DATETIME) {
            assert(len == sizeof(int64_t));
            *(int64_t *)(raw->data) = bigint_val;
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
    bool test_record(const std::vector<ColMeta> &cols, std::unique_ptr<RmRecord> &record) {
        // assert(lhs_col.tab_name == col.tab_name);
        auto lhs_col_meta = ColMeta::find_from_cols(cols, lhs_col.col_name);
        auto lhs_val = Value::read_from_record(record, lhs_col_meta);
        Value real_rhs_val;
        if (!is_rhs_val) {
            // auto cond_rhs_col_meta = tab.get_col(rhs_col.col_name);
            auto rhs_col_meta = ColMeta::find_from_cols(cols, rhs_col.col_name);
            real_rhs_val = Value::read_from_record(record, rhs_col_meta);
        } else {
            real_rhs_val = rhs_val;
        }
        bool ret = false;
        if (op == OP_EQ) {
            ret = lhs_val == real_rhs_val;
        } else if (OP_NE == op) {
            ret = lhs_val != real_rhs_val;
        } else if (OP_LT == op) {
            ret = lhs_val < real_rhs_val;
        } else if (OP_GT == op) {
            ret = lhs_val > real_rhs_val;
        } else if (OP_LE == op) {
            ret = lhs_val <= real_rhs_val;
        } else if (OP_GE == op) {
            ret = lhs_val >= real_rhs_val;
        }
        return ret;
    }
    bool test_join_record(const std::vector<ColMeta> &left_cols, std::unique_ptr<RmRecord> &left_rec,
                          const std::vector<ColMeta> &right_cols, std::unique_ptr<RmRecord> &right_rec) {
        // TODO:  交换? 比如：select * from t1,t2 on t2.id = t1.id;
        // TabCol true_lhs_col,true_rhs_col;
        // if(lhs_col.tab_name == right_tab.name){
        //     true_lhs_col = left_tab.get_col(lhs_co)
        // }

        // assert(lhs_col.tab_name ==left_tab.name);
        if (is_rhs_val) {
            return test_record(left_cols, left_rec);
        } else {
            auto cond_rhs_col_meta = ColMeta::find_from_cols(right_cols, rhs_col.col_name);
            auto real_rhs_val = Value::read_from_record(right_rec, cond_rhs_col_meta);
            auto cond_lhs_col_meta = ColMeta::find_from_cols(left_cols, lhs_col.col_name);
            auto real_lhs_val = Value::read_from_record(left_rec, cond_lhs_col_meta);
            bool ret = false;
            if (op == OP_EQ) {
                ret = real_lhs_val == real_rhs_val;
            } else if (OP_NE == op) {
                ret = real_lhs_val != real_rhs_val;
            } else if (OP_LT == op) {
                ret = real_lhs_val < real_rhs_val;
            } else if (OP_GT == op) {
                ret = real_lhs_val > real_rhs_val;
            } else if (OP_LE == op) {
                ret = real_lhs_val <= real_rhs_val;
            } else if (OP_GE == op) {
                ret = real_lhs_val >= real_rhs_val;
            }
            return ret;
        }
    }
};

struct SetClause {
    TabCol lhs;
    Value rhs;
    char op;
};

struct OrderByCaluse {
    // pair.second为true时是desc
    std::vector<std::pair<TabCol, bool>> orderby_pair;
};
struct LimitClause {
    int val;
};

inline bool type_compatible(ColType type1, ColType type2) {
    if ((type1 == TYPE_INT || type1 == TYPE_FLOAT || type1 == TYPE_BIGINT) &&
        (type2 == TYPE_INT || type2 == TYPE_FLOAT || type2 == TYPE_BIGINT)) {
        return true;
    } else if ((type1 == TYPE_STRING || type1 == TYPE_DATETIME) && (type2 == TYPE_STRING || type2 == TYPE_DATETIME)) {
        return true;
    }
    return false;
}

/*
inline int64_t S64(const char *s) {
    int64_t i;
    char c;
    int scanned = sscanf(s, "%" SCNd64 "%c", &i, &c);
    if (scanned == 1) return i;
    if (scanned > 1) {
        // TBD about extra data found
        return i;
    }
    // TBD failed to scan;
    return 0;
}
*/
