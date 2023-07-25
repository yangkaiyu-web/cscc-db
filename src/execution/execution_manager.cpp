/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "execution_manager.h"

#include "executor_delete.h"
#include "executor_index_scan.h"
#include "executor_insert.h"
#include "executor_nestedloop_join.h"
#include "executor_projection.h"
#include "executor_seq_scan.h"
#include "executor_update.h"
#include "index/ix.h"
#include "optimizer/plan.h"
#include "record_printer.h"

const char *help_info =
    "Supported SQL syntax:\n"
    "  command ;\n"
    "command:\n"
    "  CREATE TABLE table_name (column_name type [, column_name type ...])\n"
    "  DROP TABLE table_name\n"
    "  CREATE INDEX table_name (column_name)\n"
    "  DROP INDEX table_name (column_name)\n"
    "  INSERT INTO table_name VALUES (value [, value ...])\n"
    "  DELETE FROM table_name [WHERE where_clause]\n"
    "  UPDATE table_name SET column_name = value [, column_name = value ...] "
    "[WHERE where_clause]\n"
    "  SELECT selector FROM table_name [WHERE where_clause]\n"
    "type:\n"
    "  {INT | FLOAT | CHAR(n) | BIGINT | DATETIME}\n"
    "where_clause:\n"
    "  condition [AND condition ...]\n"
    "condition:\n"
    "  column op {column | value}\n"
    "column:\n"
    "  [table_name.]column_name\n"
    "op:\n"
    "  {= | <> | < | > | <= | >=}\n"
    "selector:\n"
    "  {* | column [, column ...]}\n";

// 主要负责执行DDL语句
void QlManager::run_mutli_query(std::shared_ptr<Plan> plan, Context *context) {
    if (auto x = std::dynamic_pointer_cast<DDLPlan>(plan)) {
        switch (x->tag) {
            case T_CreateTable: {
                sm_manager_->create_table(x->tab_name_, x->cols_, context);
                break;
            }
            case T_DropTable: {
                sm_manager_->drop_table(x->tab_name_, context);
                break;
            }
            case T_CreateIndex: {
                sm_manager_->create_index(x->tab_name_, x->tab_col_names_, context);
                break;
            }
            case T_DropIndex: {
                sm_manager_->drop_index(x->tab_name_, x->tab_col_names_, context);
                break;
            }
            case T_ShowIndex: {
                sm_manager_->show_indexes(x->tab_name_, context);
                break;
            }
            default:
                throw InternalError("Unexpected field type");
                break;
        }
    }
}

// 执行help; show tables; desc table; begin; commit; abort;语句
void QlManager::run_cmd_utility(std::shared_ptr<Plan> plan, txn_id_t *txn_id, Context *context) {
    if (auto x = std::dynamic_pointer_cast<OtherPlan>(plan)) {
        switch (x->tag) {
            case T_Help: {
                memcpy(context->data_send_ + *(context->offset_), help_info, strlen(help_info));
                *(context->offset_) = strlen(help_info);
                break;
            }
            case T_ShowTable: {
                sm_manager_->show_tables(context);
                break;
            }
            case T_DescTable: {
                sm_manager_->desc_table(x->tab_name_, context);
                break;
            }
            case T_Transaction_begin: {
                // 显示开启一个事务
                context->txn_->set_txn_mode(true);
                break;
            }
            case T_Transaction_commit: {
                context->txn_ = txn_mgr_->get_transaction(*txn_id);
                txn_mgr_->commit(context->txn_, context->log_mgr_);
                break;
            }
            case T_Transaction_rollback: {
                context->txn_ = txn_mgr_->get_transaction(*txn_id);
                txn_mgr_->abort(context->txn_, context->log_mgr_);
                break;
            }
            case T_Transaction_abort: {
                context->txn_ = txn_mgr_->get_transaction(*txn_id);
                txn_mgr_->abort(context->txn_, context->log_mgr_);
                break;
            }
            default:
                throw InternalError("Unexpected field type");
                break;
        }
    }
}

// 执行select语句，select语句的输出除了需要返回客户端外，还需要写入output.txt文件中
void QlManager::select_from(std::unique_ptr<AbstractExecutor> executorTreeRoot, std::vector<TabCol> sel_cols,
                            Context *context) {
    std::vector<std::string> captions;
    captions.reserve(sel_cols.size());
    // 处理表头
    for (auto &sel_col : sel_cols) {
        if (sel_col.aggregate.get() != NULL && sel_col.aggregate->aggregate_type != 4) {
            if (sel_col.aggregate->another_name != "") {
                // as 的情况
                captions.push_back(sel_col.aggregate->another_name);
            } else {
                std::string tmp_col_name;
                switch (sel_col.aggregate->aggregate_type) {
                    case 0:
                        // COUNT
                        tmp_col_name = "COUNT(";
                        tmp_col_name.append(sel_col.col_name);
                        tmp_col_name.append(")");
                        captions.push_back(tmp_col_name);
                        break;
                    case 1:
                        // MAX
                        tmp_col_name = "MAX(";
                        tmp_col_name.append(sel_col.col_name);
                        tmp_col_name.append(")");
                        captions.push_back(tmp_col_name);
                        break;
                    case 2:
                        // MIN
                        tmp_col_name = "MIN(";
                        tmp_col_name.append(sel_col.col_name);
                        tmp_col_name.append(")");
                        captions.push_back(tmp_col_name);
                        break;
                    case 3:
                        // SUM
                        tmp_col_name = "SUM(";
                        tmp_col_name.append(sel_col.col_name);
                        tmp_col_name.append(")");
                        captions.push_back(tmp_col_name);
                        break;
                    default:
                        break;
                }
            }
        } else {
            captions.push_back(sel_col.col_name);
        }
    }

    // Print header into buffer
    RecordPrinter rec_printer(sel_cols.size());
    rec_printer.print_separator(context);
    rec_printer.print_record(captions, context);
    rec_printer.print_separator(context);
    // print header into file
    std::fstream outfile;
    outfile.open("output.txt", std::ios::out | std::ios::app);
    outfile << "|";
    for (size_t i = 0; i < captions.size(); ++i) {
        outfile << " " << captions[i] << " |";
    }
    outfile << "\n";

    // Print records
    size_t num_rec = 0;

    bool has_aggregate = false;
    std::vector<std::string> tmp_item;

    // 执行query_plan
    for (executorTreeRoot->beginTuple(); !executorTreeRoot->is_end(); executorTreeRoot->nextTuple()) {
        auto Tuple = executorTreeRoot->Next();
        std::vector<std::string> columns;
        int col_id = 0;
        for (auto &col : executorTreeRoot->cols()) {
            if (sel_cols[col_id].aggregate.get() != NULL && sel_cols[col_id].aggregate->aggregate_type != 4) {
                has_aggregate = true;
            }
            std::string col_str;
            char *rec_buf = Tuple->data + col.offset;
            if (col.type == TYPE_INT) {
                col_str = std::to_string(*(int *)rec_buf);
            } else if (col.type == TYPE_BIGINT) {
                col_str = std::to_string(*(int64_t *)rec_buf);
            } else if (col.type == TYPE_FLOAT) {
                col_str = std::to_string(*(float *)rec_buf);
            } else if (col.type == TYPE_STRING) {
                col_str = std::string((char *)rec_buf, col.len);
                col_str.resize(strlen(col_str.c_str()));
            } else if (col.type == TYPE_DATETIME) {
                int64_t tmp = *(int64_t *)rec_buf;
                const std::string &year = std::to_string((tmp >> 40) & 0b1111111111111111);
                const std::string &fyear = std::string(4 - year.size(), '0') + year;

                const std::string &month = std::to_string((tmp >> 32) & 0b11111111);
                const std::string &fmonth = std::string(2 - month.size(), '0') + month;

                const std::string &day = std::to_string((tmp >> 24) & 0b11111111);
                const std::string &fday = std::string(2 - day.size(), '0') + day;

                const std::string &hour = std::to_string((tmp >> 16) & 0b11111111);
                const std::string &fhour = std::string(2 - hour.size(), '0') + hour;

                const std::string &minute = std::to_string((tmp >> 8) & 0b11111111);
                const std::string &fminute = std::string(2 - minute.size(), '0') + minute;

                const std::string &sec = std::to_string(tmp & 0b11111111);
                const std::string &fsec = std::string(2 - sec.size(), '0') + sec;

                // YYYY-MM-DD HH:MM:SS
                col_str = fyear + "-" + fmonth + "-" + fday + " " + fhour + ":" + fminute + ":" + fsec;
            }
            if (has_aggregate) {
                // 处理聚集函数
                switch (sel_cols[col_id].aggregate->aggregate_type) {
                    case 0:
                        // COUNT
                        if (col.type == TYPE_INT || col.type == TYPE_FLOAT || col.type == TYPE_STRING) {
                            if (tmp_item.size() <= col_id) {
                                tmp_item.push_back("1");
                            } else {
                                tmp_item[col_id] = std::to_string(atoi(tmp_item[col_id].c_str()) + 1);
                            }
                        } else {
                            throw AggregateParamFormatError("COUNT");
                        }
                        break;
                    case 1:
                        // MAX
                        if (tmp_item.size() <= col_id) {
                            tmp_item.push_back(col_str);
                        } else {
                            if (col.type == TYPE_INT) {
                                tmp_item[col_id] =
                                    atoi(tmp_item[col_id].c_str()) > atoi(col_str.c_str()) ? tmp_item[col_id] : col_str;
                            } else if (col.type == TYPE_FLOAT) {
                                tmp_item[col_id] =
                                    atof(tmp_item[col_id].c_str()) > atof(col_str.c_str()) ? tmp_item[col_id] : col_str;
                            } else if (col.type == TYPE_STRING) {
                                tmp_item[col_id] = tmp_item[col_id] > col_str ? tmp_item[col_id] : col_str;
                            } else {
                                throw AggregateParamFormatError("MAX");
                            }
                        }
                        break;
                    case 2:
                        // MIN
                        if (tmp_item.size() <= col_id) {
                            tmp_item.push_back(col_str);
                        } else {
                            if (col.type == TYPE_INT) {
                                tmp_item[col_id] =
                                    atoi(tmp_item[col_id].c_str()) < atoi(col_str.c_str()) ? tmp_item[col_id] : col_str;
                            } else if (col.type == TYPE_FLOAT) {
                                tmp_item[col_id] =
                                    atof(tmp_item[col_id].c_str()) < atof(col_str.c_str()) ? tmp_item[col_id] : col_str;
                            } else if (col.type == TYPE_STRING) {
                                tmp_item[col_id] = tmp_item[col_id] < col_str ? tmp_item[col_id] : col_str;
                            } else {
                                throw AggregateParamFormatError("MIN");
                            }
                        }
                        break;
                    case 3:
                        // SUM
                        if (col.type == TYPE_INT) {
                            if (tmp_item.size() <= col_id) {
                                tmp_item.push_back(col_str);
                            } else {
                                tmp_item[col_id] =
                                    std::to_string(atoi(tmp_item[col_id].c_str()) + atoi(col_str.c_str()));
                            }
                        } else if (col.type == TYPE_FLOAT) {
                            if (tmp_item.size() <= col_id) {
                                tmp_item.push_back(col_str);
                            } else {
                                tmp_item[col_id] =
                                    std::to_string(atof(tmp_item[col_id].c_str()) + atof(col_str.c_str()));
                            }
                        } else {
                            throw AggregateParamFormatError("SUM");
                        }
                        break;
                    default:
                        break;
                }
            } else {
                columns.push_back(col_str);
            }
            col_id++;
        }
        if (has_aggregate) {
            continue;
        }
        // print record into buffer
        rec_printer.print_record(columns, context);
        // print record into file
        outfile << "|";
        for (size_t i = 0; i < columns.size(); ++i) {
            outfile << " " << columns[i] << " |";
        }
        outfile << "\n";
        num_rec++;
    }
    // 打印聚集函数相关
    if (has_aggregate) {
        num_rec = 1;
        // print record into buffer
        rec_printer.print_record(tmp_item, context);
        // print record into file
        outfile << "|";
        for (int i = 0; i < tmp_item.size(); ++i) {
            outfile << " " << tmp_item[i] << " |";
        }
        outfile << "\n";
    }
    outfile.close();
    // Print footer into buffer
    rec_printer.print_separator(context);
    // Print record count into buffer
    RecordPrinter::print_record_count(num_rec, context);
}

// 执行DML语句
void QlManager::run_dml(std::unique_ptr<AbstractExecutor> exec) { exec->Next(); }
