/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "log_recovery.h"

#include <utility>

#include "common/config.h"
#include "recovery/log_defs.h"
#include "recovery/log_manager.h"
#include "system/sm_meta.h"
// #define __LOG_DEBUG
/**
 * @description: analyze阶段，需要获得脏页表（DPT）和未完成的事务列表（ATT）
 */
void RecoveryManager::analyze() {
    std::vector<TabMeta> tab_metas;

    tab_metas.reserve(sm_manager_->db_.tabs_.size());
    for (auto &it : sm_manager_->db_.tabs_) {
        tab_metas.push_back(it.second);
    }

    for (auto &tab_meta : tab_metas) {
        std::vector<ColDef> col_defs;

        sm_manager_->drop_table(tab_meta.name, nullptr);

        col_defs.reserve(tab_meta.cols.size());
        col_defs.reserve(tab_meta.cols.size());
        for (auto &col : tab_meta.cols) {
            col_defs.push_back({col.name, col.type, col.len});
        }
        sm_manager_->create_table(tab_meta.name, col_defs, nullptr);
    }
    int offset = 0;
    char log_hdr[LOG_HEADER_SIZE];
    lsn_t last_lsn = 0;
    txn_id_t last_txn = 0;
    while (int ret = disk_manager_->read_log(log_hdr, LOG_HEADER_SIZE, offset) > 0) {
        assert(ret >= 0);
        offset_list_.push_back(offset);
        LogRecord log;
        log.deserialize(log_hdr);
        lsn_offset_table_.insert(std::make_pair(log.lsn_, offset));
        lsn_prevlsn_table_.insert(std::make_pair(log.lsn_, log.prev_lsn_));
        last_lsn = log.lsn_;
        last_txn = log.log_tid_ > last_txn ? log.log_tid_ : last_txn;
#ifdef __LOG_DEBUG
        log.format_print();
#endif
        offset += log.log_tot_len_;
    }
    log_manager_->set_lsn(last_lsn + 1);
    txn_manager_->set_txn_id_num(last_txn + 1);
}

/**
 * @description: 重做所有未落盘的操作
 */
void RecoveryManager::redo() {
    int offset = 0;
    char log_hdr[LOG_HEADER_SIZE];
    while (disk_manager_->read_log(log_hdr, LOG_HEADER_SIZE, offset) > 0) {
        LogRecord log;
        log.deserialize(log_hdr);
        lsn_offset_table_.insert(std::make_pair(log.lsn_, offset));
        char *log_buf = new char[log.log_tot_len_];
        disk_manager_->read_log(log_buf, log.log_tot_len_, offset);
        offset += log.log_tot_len_;
#ifdef __LOG_DEBUG
        std::cout << "[redo] " << LogTypeStr[log.log_type_] << "\ttxn:" << log.log_tid_ << "\n";
#endif

        if (log.log_type_ == LogType::DELETE || log.log_type_ == LogType::CLR_INSERT) {
            DeleteLogRecord del_log;
            del_log.deserialize(log_buf);
            std::string tab_name(del_log.table_name_, del_log.table_name_size_);
            rollback_insert(tab_name, del_log.rid_);  //  redo delete = rollback insert;
        } else if (log.log_type_ == LogType::INSERT || log.log_type_ == LogType::CLR_DELETE) {
            InsertLogRecord insert_log;
            insert_log.deserialize(log_buf);
            std::string tab_name(insert_log.table_name_, insert_log.table_name_size_);
            rollback_delete(tab_name, insert_log.rid_, insert_log.insert_value_);
        } else if (log.log_type_ == LogType::UPDATE || log.log_type_ == LogType ::CLR_UPDATE) {
            UpdateLogRecord update_log;
            update_log.deserialize(log_buf);
            std::string tab_name(update_log.table_name_, update_log.table_name_size_);
            rollback_update(tab_name, update_log.rid_, update_log.new_value_);
        } else if (log.log_type_ == LogType::BEGIN) {
            undo_list_.insert(log.log_tid_);
        } else if (log.log_type_ == LogType::COMMIT || log.log_type_ == LogType::ABORT) {
            assert(undo_list_.find(log.log_tid_) != undo_list_.end());
            undo_list_.erase(log.log_tid_);
        }
    }
}

/**
 * @description: 回滚未完成的事务
 *
 */

void RecoveryManager::undo() {
    /*
     * NOTE: 好像只有一种情况需要 undo
     *       1. 就是 buffer pool manager 满了之后会刷盘。
     *       2. 数据库关闭时，会刷盘，这个时候没 commit 的语句需要 undo 之后再落盘。 但是这种情况感觉会很少出现。
     *       似乎不存在其他情况会刷盘。
     */

    /*
     *   一般日志对应  < lsn_t -> prev_lsn_t >
     *   clr 日志需要设置 undo next
     *   需要知道 clr 对应的 lsn_t   <clr_lsn_t -> lsn_t >
     *
     */

    for (auto offset_it = offset_list_.rbegin(); offset_it != offset_list_.rend(); ++offset_it) {
        int offset = *offset_it;
        char log_hdr[LOG_HEADER_SIZE];
        int ret = disk_manager_->read_log(log_hdr, LOG_HEADER_SIZE, offset);
        assert(ret > 0);

        LogRecord log;
        log.deserialize(log_hdr);

#ifdef __LOG_DEBUG
        std::cout << "[undo] " << LogTypeStr[log.log_type_] << "\ttxn:" << log.log_tid_ << "\n";
#endif
        if (undo_list_.find(log.log_tid_) == undo_list_.end()) {
            continue;
        }
        char *log_buf = new char[log.log_tot_len_];
        disk_manager_->read_log(log_buf, log.log_tot_len_, offset);

        if (log.log_type_ == LogType::DELETE) {
            DeleteLogRecord del_log;
            del_log.deserialize(log_buf);
            std::string tab_name(del_log.table_name_, del_log.table_name_size_);
            rollback_delete(tab_name, del_log.rid_, del_log.delete_value_);
            log_manager_->gen_log_delete_CLR(del_log.log_tid_, lsn_prevlsn_table_[del_log.lsn_], del_log.delete_value_,
                                             del_log.rid_, tab_name);

        } else if (log.log_type_ == LogType::INSERT) {
            InsertLogRecord insert_log;
            insert_log.deserialize(log_buf);
            std::string tab_name(insert_log.table_name_, insert_log.table_name_size_);
            rollback_insert(tab_name, insert_log.rid_);
            log_manager_->gen_log_insert_CLR(insert_log.log_tid_, lsn_prevlsn_table_[insert_log.prev_lsn_],
                                             insert_log.insert_value_, insert_log.rid_, tab_name);
        } else if (log.log_type_ == LogType::UPDATE) {
            UpdateLogRecord update_log;
            update_log.deserialize(log_buf);
            std::string tab_name(update_log.table_name_, update_log.table_name_size_);
            rollback_update(tab_name, update_log.rid_, update_log.old_value_);
            log_manager_->gen_log_upadte_CLR(update_log.log_tid_, lsn_prevlsn_table_[update_log.lsn_],
                                             update_log.old_value_, update_log.new_value_, update_log.rid_, tab_name);
        } else if (log.log_type_ == LogType::BEGIN) {
            undo_list_.erase(log.log_tid_);
            assert(log.prev_lsn_ == INVALID_LSN);
        } else if (log.log_type_ == LogType::COMMIT || log.log_type_ == LogType::ABORT) {
            // undo should not found
            assert(false);
        } else if (log.log_type_ == LogType::CLR_DELETE || log.log_type_ == LogType::CLR_UPDATE ||
                   log.log_type_ == LogType::CLR_INSERT) {
            // do nothing
        }
    }
    std::vector<TabMeta> tab_metas;

    tab_metas.reserve(sm_manager_->db_.tabs_.size());
    for (auto &it : sm_manager_->db_.tabs_) {
        tab_metas.push_back(it.second);
    }
    for (auto &tab_meta : tab_metas) {
        for (auto &index_meta : tab_meta.indexes) {
            std::vector<std::string> names;

            for (auto &col : index_meta.cols) {
                names.push_back(col.name);
            }
            sm_manager_->create_index(tab_meta.name, names, nullptr);
        }
    }
}

void RecoveryManager::rollback_insert(const std::string &tab_name_, const Rid &rid) {
    auto table = sm_manager_->db_.get_table(tab_name_);
    auto rec = sm_manager_->fhs_.at(tab_name_).get()->get_record(rid, nullptr);
    auto fh = sm_manager_->fhs_.at(tab_name_).get();
    for (size_t i = 0; i < table.indexes.size(); ++i) {
        auto &index = table.indexes[i];
        auto ih = sm_manager_->ihs_.at(index.get_index_name()).get();
        std::unique_ptr<RmRecord> key = std::make_unique<RmRecord>(index.col_tot_len);
        int offset = 0;
        for (size_t i = 0; i < static_cast<size_t>(index.col_num); ++i) {
            memcpy(key->data + offset, rec->data + index.cols[i].offset, index.cols[i].len);
            offset += index.cols[i].len;
        }
        ih->delete_entry(key->data, nullptr);
    }
    fh->delete_record(rid);
}
void RecoveryManager::rollback_delete(const std::string &tab_name_, const Rid &rid, const RmRecord &rec) {
    auto table = sm_manager_->db_.get_table(tab_name_);
    auto fh = sm_manager_->fhs_.at(tab_name_).get();
    for (size_t i = 0; i < table.indexes.size(); ++i) {
        auto &index = table.indexes[i];
        auto ih = sm_manager_->ihs_.at(index.get_index_name()).get();
        std::unique_ptr<RmRecord> key = std::make_unique<RmRecord>(index.col_tot_len);
        int offset = 0;
        for (size_t i = 0; i < static_cast<size_t>(index.col_num); ++i) {
            memcpy(key->data + offset, rec.data + index.cols[i].offset, index.cols[i].len);
            offset += index.cols[i].len;
        }
        ih->insert_entry(key->data, rid, nullptr);
    }
    fh->insert_record(rid, rec.data);
}
void RecoveryManager::rollback_update(const std::string &tab_name, const Rid &rid, const RmRecord &record) {
    auto table = sm_manager_->db_.get_table(tab_name);
    auto rec = sm_manager_->fhs_.at(tab_name).get()->get_record(rid, nullptr);
    auto fh = sm_manager_->fhs_.at(tab_name).get();
    for (size_t i = 0; i < table.indexes.size(); ++i) {
        auto &index = table.indexes[i];
        auto ih = sm_manager_->ihs_.at(index.get_index_name()).get();
        std::unique_ptr<RmRecord> key = std::make_unique<RmRecord>(index.col_tot_len);
        int offset = 0;
        for (size_t i = 0; i < static_cast<size_t>(index.col_num); ++i) {
            memcpy(key->data + offset, rec->data + index.cols[i].offset, index.cols[i].len);
            offset += index.cols[i].len;
        }
        ih->delete_entry(key->data, nullptr);
    }
    fh->insert_record(rid, record.data);

    for (size_t i = 0; i < table.indexes.size(); ++i) {
        auto &index = table.indexes[i];
        auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols)).get();
        std::unique_ptr<RmRecord> key = std::make_unique<RmRecord>(index.col_tot_len);
        int offset = 0;
        for (size_t i = 0; i < static_cast<size_t>(index.col_num); ++i) {
            memcpy(key->data + offset, record.data + index.cols[i].offset, index.cols[i].len);
            offset += index.cols[i].len;
        }
        ih->insert_entry(key->data, rid, nullptr);
    }
}
