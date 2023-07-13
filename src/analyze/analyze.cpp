/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "analyze.h"

#include <iomanip>

#include "common/common.h"
template <typename type>
inline bool no_overflow(const std::string &value);
int compare_int_string(const std::string &val1, const std::string &val2);
int compare_pos_int_string(const std::string &val1, const std::string &val2);

/**
 * @description:
 * 分析器，进行语义分析和查询重写，需要检查不符合语义规定的部分
 * @param {shared_ptr<ast::TreeNode>} parse parser生成的结果集
 * @return {shared_ptr<Query>} Query
 */
std::shared_ptr<Query> Analyze::do_analyze(
    std::shared_ptr<ast::TreeNode> parse) {
    std::shared_ptr<Query> query = std::make_shared<Query>();
    if (auto x = std::dynamic_pointer_cast<ast::SelectStmt>(parse)) {
        // 处理表名
        query->tables = std::move(x->tabs);
        /** TODO: 检查表是否存在 */

        // 处理target list，再target list中添加上表名，例如 a.id
        for (auto &sv_sel_col : x->cols) {
            TabCol sel_col = {.tab_name = sv_sel_col->tab_name,
                              .col_name = sv_sel_col->col_name};
            query->cols.push_back(sel_col);
        }

        std::vector<ColMeta> all_cols;
        get_all_cols(query->tables, all_cols);
        if (query->cols.empty()) {
            // select all columns
            for (auto &col : all_cols) {
                TabCol sel_col = {.tab_name = col.tab_name,
                                  .col_name = col.name};
                query->cols.push_back(sel_col);
            }
        } else {
            // infer table name from column name
            for (auto &sel_col : query->cols) {
                sel_col = check_column(all_cols, sel_col);  // 列元数据校验
            }
        }
        // 处理where条件
        get_check_clause(x->conds, query->tables, query->conds);
    } else if (auto x = std::dynamic_pointer_cast<ast::UpdateStmt>(parse)) {
        /** TODO: */

        // conds.clear();
        // for (auto &expr : sv_conds) {
        //     Condition cond;
        //     cond.lhs_col = {.tab_name = expr->lhs->tab_name, .col_name =
        //     expr->lhs->col_name}; cond.op = convert_sv_comp_op(expr->op); if
        //     (auto rhs_val = std::dynamic_pointer_cast<ast::Value>(expr->rhs))
        //     {
        //         cond.is_rhs_val = true;
        //         cond.rhs_val = convert_sv_value(rhs_val);
        //     } else if (auto rhs_col =
        //     std::dynamic_pointer_cast<ast::Col>(expr->rhs)) {
        //         cond.is_rhs_val = false;
        //         cond.rhs_col = {.tab_name = rhs_col->tab_name, .col_name =
        //         rhs_col->col_name};
        //     }
        //     conds.push_back(cond);
        // }
        std::vector<ColMeta> all_cols;
        get_all_cols({x->tab_name}, all_cols);

        query->set_clauses.clear();
        for (auto &set_clause_ptr : x->set_clauses) {
            SetClause set_clause;
            set_clause.lhs = {.tab_name = x->tab_name,
                              .col_name = set_clause_ptr->col_name};
            // Infer table name from column name
            set_clause.lhs = check_column(all_cols, set_clause.lhs);
            TabMeta &lhs_tab =
                sm_manager_->db_.get_table(set_clause.lhs.tab_name);
            auto lhs_col = lhs_tab.get_col(set_clause.lhs.col_name);
            ColType lhs_type = lhs_col->type;

            set_clause.rhs = convert_sv_value(set_clause_ptr->val, lhs_type);
            query->set_clauses.push_back(set_clause);
        }

        /*
        // Get raw values in where clause
        for (auto &set_clause : query->set_clauses) {
            // Infer table name from column name
            set_clause.lhs = check_column(all_cols, set_clause.lhs);

            TabMeta &lhs_tab =
                sm_manager_->db_.get_table(set_clause.lhs.tab_name);
            auto lhs_col = lhs_tab.get_col(set_clause.lhs.col_name);
            ColType lhs_type = lhs_col->type;
            ColType rhs_type;

            if (lhs_type != set_clause.rhs.type) {
                set_clause.rhs.cast_to(lhs_type);
            }
            rhs_type = set_clause.rhs.type;

            if (lhs_type != rhs_type) {
                throw IncompatibleTypeError(coltype2str(lhs_type),
                                            coltype2str(rhs_type));
            }
        }*/

        get_check_clause(x->conds, {x->tab_name}, query->conds);

    } else if (auto x = std::dynamic_pointer_cast<ast::DeleteStmt>(parse)) {
        // 处理where条件
        get_check_clause(x->conds, {x->tab_name}, query->conds);
    } else if (auto x = std::dynamic_pointer_cast<ast::InsertStmt>(parse)) {
        std::vector<ColMeta> all_cols;
        get_all_cols({x->tab_name}, all_cols);
        // 检查
        assert(all_cols.size() == x->vals.size());
        // 处理insert 的values值
        for (size_t i = 0; i < all_cols.size(); ++i) {
            TabMeta &tab = sm_manager_->db_.get_table(x->tab_name);
            auto col = tab.get_col(all_cols[i].name);
            query->values.push_back(convert_sv_value(x->vals[i], col->type));
        }
    } else {
        // do nothing
    }
    query->parse = std::move(parse);
    return query;
}

// Infer table name from column name
TabCol Analyze::check_column(const std::vector<ColMeta> &all_cols,
                             TabCol target) {
    if (target.tab_name.empty()) {
        // Table name not specified, infer table name from column name
        std::string tab_name;
        for (auto &col : all_cols) {
            if (col.name == target.col_name) {
                if (!tab_name.empty()) {
                    throw AmbiguousColumnError(target.col_name);
                }
                tab_name = col.tab_name;
            }
        }
        if (tab_name.empty()) {
            throw ColumnNotFoundError(target.col_name);
        }
        target.tab_name = tab_name;
    } else {
        /** TODO: Make sure target column exists */
    }
    return target;
}

void Analyze::get_all_cols(const std::vector<std::string> &tab_names,
                           std::vector<ColMeta> &all_cols) {
    for (auto &sel_tab_name : tab_names) {
        // 这里db_不能写成get_db(), 注意要传指针
        const auto &sel_tab_cols =
            sm_manager_->db_.get_table(sel_tab_name).cols;
        all_cols.insert(all_cols.end(), sel_tab_cols.begin(),
                        sel_tab_cols.end());
    }
}

void Analyze::get_check_clause(
    const std::vector<std::shared_ptr<ast::BinaryExpr>> &sv_conds,
    const std::vector<std::string> &tab_names, std::vector<Condition> &conds) {
    conds.clear();
    std::vector<ColMeta> all_cols;
    get_all_cols(tab_names, all_cols);

    for (auto &expr : sv_conds) {
        Condition cond;
        // 运算符
        cond.op = convert_sv_comp_op(expr->op);
        // 运算符左边列
        cond.lhs_col = {.tab_name = expr->lhs->tab_name,
                        .col_name = expr->lhs->col_name};
        // Infer table name from column name
        cond.lhs_col = check_column(all_cols, cond.lhs_col);
        TabMeta &lhs_tab = sm_manager_->db_.get_table(cond.lhs_col.tab_name);
        auto lhs_col = lhs_tab.get_col(cond.lhs_col.col_name);
        ColType lhs_type = lhs_col->type;
        ColType rhs_type;
        if (auto rhs_val = std::dynamic_pointer_cast<ast::Value>(expr->rhs)) {
            cond.is_rhs_val = true;
            cond.rhs_val = convert_sv_value(rhs_val, lhs_type);
        } else if (auto rhs_col =
                       std::dynamic_pointer_cast<ast::Col>(expr->rhs)) {
            cond.is_rhs_val = false;
            cond.rhs_col = {.tab_name = rhs_col->tab_name,
                            .col_name = rhs_col->col_name};
            // Infer table name from column name
            cond.rhs_col = check_column(all_cols, cond.rhs_col);
            TabMeta &rhs_tab =
                sm_manager_->db_.get_table(cond.rhs_col.tab_name);
            rhs_type = rhs_tab.get_col(cond.rhs_col.col_name)->type;
            if (!type_compatible(lhs_type, rhs_type)) {
                throw CastTypeError(coltype2str(lhs_type),
                                    coltype2str(rhs_type));
            }
        }
        conds.push_back(cond);
    }
}
/*
// 处理二元关系表达式
void Analyze::get_clause(
    const std::vector<std::shared_ptr<ast::BinaryExpr>> &sv_conds,
    std::vector<Condition> &conds) {
    conds.clear();
    for (auto &expr : sv_conds) {
        Condition cond;
        cond.lhs_col = {.tab_name = expr->lhs->tab_name,
                        .col_name = expr->lhs->col_name};
        cond.op = convert_sv_comp_op(expr->op);
        if (auto rhs_val = std::dynamic_pointer_cast<ast::Value>(expr->rhs)) {
            cond.is_rhs_val = true;
            cond.rhs_val = convert_sv_value(rhs_val);
        } else if (auto rhs_col =
                       std::dynamic_pointer_cast<ast::Col>(expr->rhs)) {
            cond.is_rhs_val = false;
            cond.rhs_col = {.tab_name = rhs_col->tab_name,
                            .col_name = rhs_col->col_name};
        }
        conds.push_back(cond);
    }
}

void Analyze::check_clause(const std::vector<std::string> &tab_names,
                           std::vector<Condition> &conds) {
    // auto all_cols = get_all_cols(tab_names);
    std::vector<ColMeta> all_cols;
    get_all_cols(tab_names, all_cols);
    // Get raw values in where clause
    for (auto &cond : conds) {
        // Infer table name from column name
        cond.lhs_col = check_column(all_cols, cond.lhs_col);
        if (!cond.is_rhs_val) {
            cond.rhs_col = check_column(all_cols, cond.rhs_col);
        }
        TabMeta &lhs_tab = sm_manager_->db_.get_table(cond.lhs_col.tab_name);
        auto lhs_col = lhs_tab.get_col(cond.lhs_col.col_name);
        ColType lhs_type = lhs_col->type;
        ColType rhs_type;
        if (cond.is_rhs_val) {
            if (lhs_type != cond.rhs_val.type) {
                cond.rhs_val.cast_to(lhs_type);
            }
            // cond.rhs_val.init_raw(lhs_col->len);
            rhs_type = cond.rhs_val.type;
        } else {
            TabMeta &rhs_tab =
                sm_manager_->db_.get_table(cond.rhs_col.tab_name);
            auto rhs_col = rhs_tab.get_col(cond.rhs_col.col_name);
            rhs_type = rhs_col->type;
        }
        if (lhs_type != rhs_type) {
            throw IncompatibleTypeError(coltype2str(lhs_type),
                                        coltype2str(rhs_type));
        }
    }
}
*/

// 有点冗长
Value Analyze::convert_sv_value(const std::shared_ptr<ast::Value> &sv_val,
                                ColType sv_cast_to_type) {
    Value val;
    if (auto int_bint_lit =
            std::dynamic_pointer_cast<ast::Int_Bint_Lit>(sv_val)) {
        const std::string &int_bint = int_bint_lit->val;
        if (sv_cast_to_type == TYPE_BIGINT) {
            if (no_overflow<int64_t>(int_bint)) {
                val.set_bigint(std::stoll(int_bint));
            } else {
                throw BigIntOverflowError(int_bint_lit->val);
            }
        } else if (sv_cast_to_type == TYPE_INT) {
            // 检查范围防止溢出
            if (no_overflow<int>(int_bint)) {
                val.set_int(static_cast<int>(std::stoi(int_bint)));
            } else {
                throw IntOverflowError(int_bint_lit->val);
            }
        } else if (sv_cast_to_type == TYPE_FLOAT) {
            if (no_overflow<float>(int_bint)) {
                val.set_float((static_cast<float>(std::stof(int_bint))));
            } else {
                throw FloatOverflowError(int_bint_lit->val);
            }
        } else {
            throw CastTypeError("int_bigint", coltype2str(sv_cast_to_type));
        }
    } else if (auto float_lit =
                   std::dynamic_pointer_cast<ast::FloatLit>(sv_val)) {
        if (sv_cast_to_type == TYPE_FLOAT) {
            val.set_float(float_lit->val);
        } else if (sv_cast_to_type == TYPE_INT) {
            val.set_int(static_cast<int>(float_lit->val));
        } else if (sv_cast_to_type == TYPE_BIGINT) {
            val.set_bigint(static_cast<int64_t>(float_lit->val));
        } else {
            throw CastTypeError("float", coltype2str(sv_cast_to_type));
        }
    } else if (auto str_lit =
                   std::dynamic_pointer_cast<ast::StringLit>(sv_val)) {
        if (sv_cast_to_type == TYPE_STRING) {
            val.set_str(str_lit->val);
        } else {
            throw CastTypeError("string", coltype2str(sv_cast_to_type));
        }
    } else {
        throw InternalError("Unexpected sv value type");
    }
    return val;
}

CompOp Analyze::convert_sv_comp_op(ast::SvCompOp op) {
    std::map<ast::SvCompOp, CompOp> m = {
        {ast::SV_OP_EQ, OP_EQ}, {ast::SV_OP_NE, OP_NE}, {ast::SV_OP_LT, OP_LT},
        {ast::SV_OP_GT, OP_GT}, {ast::SV_OP_LE, OP_LE}, {ast::SV_OP_GE, OP_GE},
    };
    return m.at(op);
}

// util functtion
// 相等返回0，val1大于val2返回1，小于返回-1
int compare_pos_int_string(const std::string &val1, const std::string &val2) {
    int len1 = val1.size(), len2 = val2.size();
    if (len1 > len2) {
        return 1;
    } else if (len1 < len2) {
        return -1;
    } else {
        for (int i = 0; i < val1.size(); ++i) {
            if (val1[i] > val2[i]) {
                return 1;
            } else if (val1[i] < val2[i]) {
                return -1;
            }
        }
        return 0;
    }
}

// 相等返回0，val1大于val2返回1，小于返回-1
int compare_int_string(const std::string &val1, const std::string &val2) {
    assert(!val1.empty() && !val2.empty());

    if (val2[0] == '-' && val1[0] != '-') {
        return 1;
    } else if (val1[0] == '-' && val2[0] != '-') {
        return -1;
    } else if (val1[0] == '-' && val2[0] == '-') {
        return -compare_pos_int_string(val1.substr(1), val2.substr(1));
    } else {
        return compare_pos_int_string(val1[0] == '+' ? val1.substr(1) : val1,
                                      val2[0] == '+' ? val2.substr(1) : val2);
    }
}

template <typename type>
inline bool no_overflow(const std::string &value) {
    return (compare_int_string(
                value, std::to_string(std::numeric_limits<type>::max())) <=
            0) &&
           (compare_int_string(
                value, std::to_string(std::numeric_limits<type>::min())) >= 0);
}