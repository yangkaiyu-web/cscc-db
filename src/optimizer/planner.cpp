/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "planner.h"

#include <algorithm>
#include <memory>

#include "common/common.h"
#include "defs.h"
#include "errors.h"
#include "execution/executor_delete.h"
#include "execution/executor_index_scan.h"
#include "execution/executor_insert.h"
#include "execution/executor_nestedloop_join.h"
#include "execution/executor_projection.h"
#include "execution/executor_seq_scan.h"
#include "execution/executor_update.h"
#include "index/ix.h"
#include "optimizer/plan.h"
#include "record_printer.h"

// 目前的索引匹配规则为：自动调整where条件的顺序
bool Planner::get_index_cols(std::string tab_name, std::vector<Condition> &curr_conds,
                             std::vector<Condition> &idx_conds, std::vector<std::string> &index_col_names) {
    // 如果存在右边不是字面量的，不能使用索引，当前仅支持右边字面量使用索引
    for (auto &cond : curr_conds) {
        if (cond.is_rhs_val) {
            idx_conds.push_back(cond);
        }
    }
    if (idx_conds.size() == 0) return false;

    index_col_names.clear();
    TabMeta &tab = sm_manager_->db_.get_table(tab_name);

    // 对表上建立的所有索引进行搜索，看哪一个索引"从最左边列"开始"连续"与fed_conds中的列相匹配
    bool found = false;
    for (auto &index : tab.indexes) {
        // 保存与index连续匹配的conds
        std::vector<Condition> tmp_conds;
        for (int i = 0; i < index.col_num; ++i) {
            // conds中是否有与index中第i列相匹配的列
            bool flag = false;
            for (size_t j = 0; j < idx_conds.size(); ++j) {
                if (idx_conds[j].lhs_col.col_name.compare(index.cols[i].name) == 0) {
                    flag = true;
                    tmp_conds.push_back(idx_conds[j]);
                }
            }
            // 如果没有，就不能继续连续匹配下去了
            if (!flag) {
                break;
            }
        }
        // 完全没有连续匹配的conds，则考虑表中下一个索引
        if (tmp_conds.size() == 0) continue;
        // 否则认为找到了，保存到fed conds
        tmp_conds.swap(idx_conds);
        found = true;
        for (auto &col_meta : index.cols) {
            index_col_names.push_back(col_meta.name);
        }
        break;
    }
    if (found) {
        return true;
    }
    return false;
}

/**
 * @brief 表算子条件谓词生成
 *
 * @param conds 条件
 * @param tab_names 表名
 * @return std::vector<Condition>
 */
std::vector<Condition> pop_conds(std::vector<Condition> &conds, std::string tab_names) {
    // auto has_tab = [&](const std::string &tab_name) {
    //     return std::find(tab_names.begin(), tab_names.end(), tab_name) !=
    //     tab_names.end();
    // };
    std::vector<Condition> solved_conds;
    auto it = conds.begin();
    while (it != conds.end()) {
        if ((tab_names.compare(it->lhs_col.tab_name) == 0 && it->is_rhs_val) ||
            (it->lhs_col.tab_name.compare(it->rhs_col.tab_name) == 0)) {
            solved_conds.emplace_back(std::move(*it));
            it = conds.erase(it);
        } else {
            it++;
        }
    }
    return solved_conds;
}

int push_conds(Condition *cond, std::shared_ptr<Plan> plan) {
    if (auto x = std::dynamic_pointer_cast<ScanPlan>(plan)) {
        if (x->tab_name_.compare(cond->lhs_col.tab_name) == 0) {
            return 1;
        } else if (x->tab_name_.compare(cond->rhs_col.tab_name) == 0) {
            return 2;
        } else {
            return 0;
        }
    } else if (auto x = std::dynamic_pointer_cast<JoinPlan>(plan)) {
        int left_res = push_conds(cond, x->left_);
        // 条件已经下推到左子节点
        if (left_res == 3) {
            return 3;
        }
        int right_res = push_conds(cond, x->right_);
        // 条件已经下推到右子节点
        if (right_res == 3) {
            return 3;
        }
        // 左子节点或右子节点有一个没有匹配到条件的列
        if (left_res == 0 || right_res == 0) {
            return left_res + right_res;
        }
        // 左子节点匹配到条件的右边
        if (left_res == 2) {
            // 需要将左右两边的条件变换位置
            std::map<CompOp, CompOp> swap_op = {
                {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
            };
            std::swap(cond->lhs_col, cond->rhs_col);
            cond->op = swap_op.at(cond->op);
        }
        x->conds_.emplace_back(std::move(*cond));
        return 3;
    }
    return false;
}

std::shared_ptr<Plan> pop_scan(int *scantbl, std::string table, std::vector<std::string> &joined_tables,
                               std::vector<std::shared_ptr<Plan>> plans) {
    for (size_t i = 0; i < plans.size(); i++) {
        auto x = std::dynamic_pointer_cast<ScanPlan>(plans[i]);
        if (x->tab_name_.compare(table) == 0) {
            scantbl[i] = 1;
            joined_tables.emplace_back(x->tab_name_);
            return plans[i];
        }
    }
    return nullptr;
}

std::shared_ptr<Query> Planner::logical_optimization(std::shared_ptr<Query> query, Context *context) {
    // TODO 实现逻辑优化规则

    return query;
}

std::shared_ptr<Plan> Planner::physical_optimization(std::shared_ptr<Query> query, Context *context) {
    std::shared_ptr<Plan> plan = make_one_rel(query);

    // 其他物理优化

    // 处理orderby
    plan = generate_sort_plan(query, std::move(plan));

    return plan;
}

std::shared_ptr<Plan> Planner::make_one_rel(std::shared_ptr<Query> query) {
    auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    std::vector<std::string> tables = query->tables;
    // // Scan table , 生成表算子列表tab_nodes
    std::vector<std::shared_ptr<Plan>> table_scan_executors(tables.size());
    for (size_t i = 0; i < tables.size(); i++) {
        auto curr_conds = pop_conds(query->conds, tables[i]);
        // int index_no = get_indexNo(tables[i], curr_conds);
        // 真正的参与索引查找的列
        std::vector<Condition> idx_conds;
        std::vector<std::string> index_col_names;
        bool index_exist = get_index_cols(tables[i], curr_conds, idx_conds, index_col_names);
        if (index_exist == false) {  // 该表没有索引
            index_col_names.clear();
            table_scan_executors[i] =
                std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, tables[i], curr_conds, idx_conds, index_col_names);
        } else {  // 存在索引
            table_scan_executors[i] =
                std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, tables[i], curr_conds, idx_conds, index_col_names);
        }
    }
    // 只有一个表，不需要join。
    if (tables.size() == 1) {
        return table_scan_executors[0];
    }
    // 获取where条件
    auto conds = std::move(query->conds);
    std::shared_ptr<Plan> table_join_executors;

    int scantbl[tables.size()];
    for (size_t i = 0; i < tables.size(); i++) {
        scantbl[i] = -1;
    }
    // 假设在ast中已经添加了jointree，这里需要修改的逻辑是，先处理jointree，然后再考虑剩下的部分
    if (conds.size() >= 1) {
        // 有连接条件

        // 根据连接条件，生成第一层join
        std::vector<std::string> joined_tables(tables.size());
        auto it = conds.begin();
        while (it != conds.end()) {
            std::shared_ptr<Plan> left, right;
            left = pop_scan(scantbl, it->lhs_col.tab_name, joined_tables, table_scan_executors);
            right = pop_scan(scantbl, it->rhs_col.tab_name, joined_tables, table_scan_executors);
            std::vector<Condition> join_conds{*it};
            // 建立join
            table_join_executors =
                std::make_shared<JoinPlan>(T_NestLoop, std::move(left), std::move(right), join_conds);
            it = conds.erase(it);
            break;
        }
        // 根据连接条件，生成第2-n层join
        it = conds.begin();
        while (it != conds.end()) {
            std::shared_ptr<Plan> left_need_to_join_executors = nullptr;
            std::shared_ptr<Plan> right_need_to_join_executors = nullptr;
            bool isneedreverse = false;
            if (std::find(joined_tables.begin(), joined_tables.end(), it->lhs_col.tab_name) == joined_tables.end()) {
                left_need_to_join_executors =
                    pop_scan(scantbl, it->lhs_col.tab_name, joined_tables, table_scan_executors);
            }
            if (std::find(joined_tables.begin(), joined_tables.end(), it->rhs_col.tab_name) == joined_tables.end()) {
                right_need_to_join_executors =
                    pop_scan(scantbl, it->rhs_col.tab_name, joined_tables, table_scan_executors);
                isneedreverse = true;
            }

            if (left_need_to_join_executors != nullptr && right_need_to_join_executors != nullptr) {
                std::vector<Condition> join_conds{*it};
                std::shared_ptr<Plan> temp_join_executors =
                    std::make_shared<JoinPlan>(T_NestLoop, std::move(left_need_to_join_executors),
                                               std::move(right_need_to_join_executors), join_conds);
                table_join_executors =
                    std::make_shared<JoinPlan>(T_NestLoop, std::move(temp_join_executors),
                                               std::move(table_join_executors), std::vector<Condition>());
            } else if (left_need_to_join_executors != nullptr || right_need_to_join_executors != nullptr) {
                if (isneedreverse) {
                    std::map<CompOp, CompOp> swap_op = {
                        {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
                    };
                    std::swap(it->lhs_col, it->rhs_col);
                    it->op = swap_op.at(it->op);
                    left_need_to_join_executors = std::move(right_need_to_join_executors);
                }
                std::vector<Condition> join_conds{*it};
                table_join_executors = std::make_shared<JoinPlan>(T_NestLoop, std::move(left_need_to_join_executors),
                                                                  std::move(table_join_executors), join_conds);
            } else {
                push_conds(std::move(&(*it)), table_join_executors);
            }
            it = conds.erase(it);
        }
    } else {
        table_join_executors = table_scan_executors[0];
        scantbl[0] = 1;
    }

    // 连接剩余表
    for (size_t i = 0; i < tables.size(); i++) {
        if (scantbl[i] == -1) {
            table_join_executors =
                std::make_shared<JoinPlan>(T_NestLoop, std::move(table_scan_executors[i]),
                                           std::move(table_join_executors), std::vector<Condition>());
        }
    }

    return table_join_executors;
}

std::shared_ptr<Plan> Planner::generate_agg_plan(std::shared_ptr<Query> query,
                                                  std::shared_ptr<Plan> plan) {
    bool has_agg=false;
    for(auto&col :query->cols){
        if(col.has_agg){
            has_agg=true;
            // TODO: 这里怎么检测非法情况，因为不支持group by,所以col list里，只能由一个聚集函数且普通列名不能同时出现
            // bool ok = 
            // if(query->cols.size()!=1){
            //     throw InternalError("not support cols and agg like 'select a,count(b) from t1;'");
            // }
        }
    }
    if (!has_agg) {
        return plan;
    }
    auto col = query->cols[0];
    return std::make_shared<AggPlan>(
        T_Sort, std::move(plan), col);
}
std::shared_ptr<Plan> Planner::generate_sort_plan(std::shared_ptr<Query> query,
                                                  std::shared_ptr<Plan> plan) {
    auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (!x->has_sort) {
        return plan;
    }
    int limit_num = -1;
    if (x->has_limit) {
        limit_num = query->limit.val;
    }
    return std::make_shared<SortPlan>(T_Sort, std::move(plan), query->oder_by.orderby_pair, limit_num);
}

/**
 * @brief select plan 生成
 *
 * @param sel_cols select plan 选取的列
 * @param tab_names select plan 目标的表
 * @param conds select plan 选取条件
 */
std::shared_ptr<Plan> Planner::generate_select_plan(std::shared_ptr<Query> query, Context *context) {
    // 逻辑优化
    query = logical_optimization(std::move(query), context);

    // 物理优化
    auto sel_cols = query->cols;
    std::shared_ptr<Plan> plannerRoot = physical_optimization(query, context);

    // handle count(*)
    // TODO:有更好的方式
    if(sel_cols.size()==1 && sel_cols[0].col_name=="" && sel_cols[0].has_agg &&
        sel_cols[0].aggregate_type==AggType::COUNT){
        sel_cols.clear();
        std::vector<ColMeta> all_cols;
        for (const auto &sel_tab_name :query->tables) {
            // 这里db_不能写成get_db(), 注意要传指针
            const auto &sel_tab_cols = sm_manager_->db_.get_table(sel_tab_name).cols;
            for(const auto& col : sel_tab_cols){
                TabCol sel_col = {.tab_name = col.tab_name, .col_name = col.name};
                sel_cols.emplace_back(sel_col);
            }

        }
    }
    plannerRoot = std::make_shared<ProjectionPlan>(
        T_Projection, std::move(plannerRoot), std::move(sel_cols));
    // agg
    plannerRoot= generate_agg_plan(query, std::move(plannerRoot));

    return plannerRoot;
}

// 生成DDL语句和DML语句的查询执行计划
std::shared_ptr<Plan> Planner::do_planner(std::shared_ptr<Query> query, Context *context) {
    std::shared_ptr<Plan> plannerRoot;
    if (auto x = std::dynamic_pointer_cast<ast::CreateTable>(query->parse)) {
        // create table;
        std::vector<ColDef> col_defs;
        for (auto &field : x->fields) {
            if (auto sv_col_def = std::dynamic_pointer_cast<ast::ColDef>(field)) {
                ColDef col_def = {.name = sv_col_def->col_name,
                                  .type = interp_sv_type(sv_col_def->type_len->type),
                                  .len = sv_col_def->type_len->len};
                col_defs.push_back(col_def);
            } else {
                throw InternalError("Unexpected field type");
            }
        }
        plannerRoot = std::make_shared<DDLPlan>(T_CreateTable, x->tab_name, std::vector<std::string>(), col_defs);
    } else if (auto x = std::dynamic_pointer_cast<ast::DropTable>(query->parse)) {
        // drop table;
        plannerRoot =
            std::make_shared<DDLPlan>(T_DropTable, x->tab_name, std::vector<std::string>(), std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::CreateIndex>(query->parse)) {
        // create index;
        plannerRoot = std::make_shared<DDLPlan>(T_CreateIndex, x->tab_name, x->col_names, std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::DropIndex>(query->parse)) {
        // drop index
        plannerRoot = std::make_shared<DDLPlan>(T_DropIndex, x->tab_name, x->col_names, std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::ShowIndexes>(query->parse)) {
        // show tables;
        return std::make_shared<DDLPlan>(T_ShowIndex, x->tab_name, std::vector<std::string>(), std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::InsertStmt>(query->parse)) {
        // insert;
        plannerRoot = std::make_shared<DMLPlan>(T_Insert, std::shared_ptr<Plan>(), x->tab_name, query->values,
                                                query->conds, query->set_clauses);
    } else if (auto x = std::dynamic_pointer_cast<ast::DeleteStmt>(query->parse)) {
        // delete;
        // 生成表扫描方式
        std::shared_ptr<Plan> table_scan_executors;
        // 只有一张表，不需要进行物理优化了
        // int index_no = get_indexNo(x->tab_name, query->conds);
        std::vector<std::string> index_col_names;
        std::vector<Condition> idx_conds;
        bool index_exist = get_index_cols(x->tab_name, query->conds, idx_conds, index_col_names);

        if (index_exist == false) {  // 该表没有索引
            index_col_names.clear();
            table_scan_executors = std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, x->tab_name, query->conds,
                                                              idx_conds, index_col_names);
        } else {  // 存在索引
            table_scan_executors = std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, x->tab_name, query->conds,
                                                              idx_conds, index_col_names);
        }

        plannerRoot = std::make_shared<DMLPlan>(T_Delete, table_scan_executors, x->tab_name, std::vector<Value>(),
                                                query->conds, std::vector<SetClause>());
    } else if (auto x = std::dynamic_pointer_cast<ast::UpdateStmt>(query->parse)) {
        // update;
        // 生成表扫描方式
        std::shared_ptr<Plan> table_scan_executors;
        // 只有一张表，不需要进行物理优化了
        // int index_no = get_indexNo(x->tab_name, query->conds);
        std::vector<std::string> index_col_names;
        std::vector<Condition> idx_conds;
        bool index_exist = get_index_cols(x->tab_name, query->conds, idx_conds, index_col_names);

        if (index_exist == false) {  // 该表没有索引
            index_col_names.clear();
            table_scan_executors = std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, x->tab_name, query->conds,
                                                              idx_conds, index_col_names);
        } else {  // 存在索引
            table_scan_executors = std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, x->tab_name, query->conds,
                                                              idx_conds, index_col_names);
        }
        plannerRoot = std::make_shared<DMLPlan>(T_Update, table_scan_executors, x->tab_name, std::vector<Value>(),
                                                query->conds, query->set_clauses);
    } else if (auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse)) {
        std::shared_ptr<plannerInfo> root = std::make_shared<plannerInfo>(x);
        // 生成select语句的查询执行计划
        std::shared_ptr<Plan> projection = generate_select_plan(std::move(query), context);
        plannerRoot = std::make_shared<DMLPlan>(T_select, projection, std::string(), std::vector<Value>(),
                                                std::vector<Condition>(), std::vector<SetClause>());
    } else {
        throw InternalError("Unexpected AST root");
    }
    return plannerRoot;
}
