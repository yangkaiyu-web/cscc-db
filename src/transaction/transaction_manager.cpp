/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "transaction_manager.h"

std::unordered_map<txn_id_t, Transaction *> TransactionManager::txn_map = {};

/**
 * @description: 事务的开始方法
 * @return {Transaction*} 开始事务的指针
 * @param {Transaction*} txn 事务指针，空指针代表需要创建新事务，否则开始已有事务
 * @param {LogManager*} log_manager 日志管理器指针
 */
Transaction *TransactionManager::begin(Transaction *txn, LogManager *log_manager) {
    // Todo:
    // 1. 判断传入事务参数是否为空指针
    // 2. 如果为空指针，创建新事务
    // 3. 把开始事务加入到全局事务表中
    // 4. 返回当前事务指针
    if (txn == nullptr) {
        auto txn_id = next_txn_id_++;
        txn = new Transaction(txn_id);
        txn->set_start_ts(next_timestamp_++);
        txn->set_state(TransactionState::DEFAULT);
        txn_map[txn_id] = txn;
        LogRecord *log_record =
            new BeginLogRecord(log_manager->alloc_lsn(), txn->get_transaction_id(), txn->get_so_far_lsn());
        txn->set_so_far_lsn(log_record->lsn_);
        return txn;
    } else {
        // TODO:目前还不知道怎么处理
        assert(false);
    }
    return txn;
}

/**
 * @description: 事务的提交方法
 * @param {Transaction*} txn 需要提交的事务
 * @param {LogManager*} log_manager 日志管理器指针
 */
void TransactionManager::commit(Transaction *txn, LogManager *log_manager) {
    // Todo:
    // 1. 如果存在未提交的写操作，提交所有的写操作
    // 2. 释放所有锁
    // 3. 释放事务相关资源，eg.锁集
    // 4. 把事务日志刷入磁盘中
    // 5. 更新事务状态
    assert(txn != nullptr);
    txn->set_state(TransactionState::COMMITTED);
    // TODO:提交写操作(写日志？)
    auto write_records = txn->get_write_records();
    auto write_indexes = txn->get_write_indexes();
    auto index_deleted_page_set = txn->get_index_deleted_page_set();
    auto index_latch_page_set = txn->get_index_latch_page_set();
    auto lock_set = txn->get_lock_set();

    for (auto lock : *lock_set) {
        lock_manager_->unlock(txn, lock);
    }
    write_records->clear();
    write_indexes->clear();
    index_deleted_page_set->clear();
    index_latch_page_set->clear();
    lock_set->clear();

    LogRecord *log_record =
        new CommitLogRecord(log_manager->alloc_lsn(), txn->get_transaction_id(), txn->get_so_far_lsn());
    txn->set_so_far_lsn(log_record->lsn_);
    log_manager->add_log_to_buffer(log_record);
    log_manager->flush_log_to_disk();
}

/**
 * @description: 事务的终止（回滚）方法
 * @param {Transaction *} txn 需要回滚的事务
 * @param {LogManager} *log_manager 日志管理器指针
 */
void TransactionManager::abort(Transaction *txn, LogManager *log_manager) {
    // Todo:
    // 1. 回滚所有写操作
    // 2. 释放所有锁
    // 3. 清空事务相关资源，eg.锁集
    // 4. 把事务日志刷入磁盘中
    // 5. 更新事务状态
    assert(txn != nullptr);
    txn->set_state(TransactionState::ABORTED);
    auto write_records = txn->get_write_records();
    while (!write_records->empty()) {
        switch (write_records->back()->GetWriteType()) {
            case WType::INSERT_TUPLE:
                rollback_insert_record(write_records->back()->GetHdl(), write_records->back()->GetRid(), txn);
                break;
            case WType::DELETE_TUPLE:
                rollback_delete_record(write_records->back()->GetHdl(), write_records->back()->GetRid(),
                                       write_records->back()->GetRecord(), txn);
                break;
            case WType::UPDATE_TUPLE:
                rollback_update_record(write_records->back()->GetHdl(), write_records->back()->GetRid(),
                                       write_records->back()->GetRecord(), txn);
                break;
        }
        write_records->pop_back();
    }

    auto write_indexes = txn->get_write_indexes();
    while (!write_indexes->empty()) {
        switch (write_indexes->back()->GetWriteType()) {
            case WType::INSERT_INDEX:
                rollback_insert_index(write_indexes->back()->GetHdl(), write_indexes->back()->MoveOldKey(), txn);
                break;
            case WType::DELETE_INDEX:
                rollback_delete_index(write_indexes->back()->GetHdl(), write_indexes->back()->GetRid(),
                                      write_indexes->back()->MoveOldKey(), txn);
                break;
            case WType::UPDATE_INDEX:
                rollback_update_index(write_indexes->back()->GetHdl(), write_indexes->back()->GetRid(),
                                      write_indexes->back()->MoveOldKey(), write_indexes->back()->MoveNewKey(), txn);
                break;
        }
        write_indexes->pop_back();
    }
    auto lock_set = txn->get_lock_set();
    for (auto lock : *lock_set) {
        lock_manager_->unlock(txn, lock);
    }
    write_records->clear();
    write_indexes->clear();
    lock_set->clear();

    LogRecord *log_record =
        new AbortLogRecord(log_manager->alloc_lsn(), txn->get_transaction_id(), txn->get_so_far_lsn());
    txn->set_so_far_lsn(log_record->lsn_);
    log_manager->add_log_to_buffer(log_record);
    log_manager->flush_log_to_disk();
    txn->set_state(TransactionState::ABORTED);
}

/**
 * @description: 回滚插入的方法
 * @param tab_name_ 表名
 * @param rid
 * @param txn 需要回滚的事务
 */
void TransactionManager::rollback_insert_record(RmFileHandle *rm_hdl, const Rid &rid, Transaction *txn) {
    rm_hdl->delete_record(rid, nullptr);
}

/**
 * @description: 回滚删除的方法
 * @param tab_name_ 表名
 * @param rid
 * @param txn 需要回滚的事务
 */
void TransactionManager::rollback_delete_record(RmFileHandle *rm_hdl, const Rid &rid, const RmRecord &record,
                                                Transaction *txn) {
    rm_hdl->insert_record(rid, record.data);
}

/**
 * @description: 回滚更新的方法
 * @param tab_name_ 表名
 * @param rid
 * @param record 记录
 * @param txn 需要回滚的事务
 */
void TransactionManager::rollback_update_record(RmFileHandle *rm_hdl, const Rid &rid, const RmRecord &record,
                                                Transaction *txn) {
    rm_hdl->insert_record(rid, record.data);
}

void TransactionManager::rollback_insert_index(IxIndexHandle *idx_hdl, std::unique_ptr<char[]> key, Transaction *txn) {
    idx_hdl->delete_entry(key.get(), txn);
}

void TransactionManager::rollback_delete_index(IxIndexHandle *idx_hdl, const Rid &rid, std::unique_ptr<char[]> key,
                                               Transaction *txn) {
    idx_hdl->insert_entry(key.get(), rid, txn);
}

void TransactionManager::rollback_update_index(IxIndexHandle *idx_hdl, const Rid &rid, std::unique_ptr<char[]> old_key,
                                               std::unique_ptr<char[]> new_key, Transaction *txn) {
    idx_hdl->delete_entry(old_key.get(), txn);
    idx_hdl->insert_entry(new_key.get(), rid, txn);
}
