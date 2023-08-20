/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "lock_manager.h"

#include "transaction/txn_defs.h"

/**
 * @description: 申请行级共享锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID 记录所在的表的fd
 * @param {int} tab_fd
 *
 *   rid
 *        S -> S-> S -> X
 *
 */

bool LockManager::lock_shared_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    if (txn->get_state() == TransactionState::ABORTED) {
        return false;
    }
    if (txn->get_state() == TransactionState::SHRINKING) {
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    std::scoped_lock<std::mutex> latch(latch_);
    txn_table_[txn->get_transaction_id()] = txn;
    auto lock_data_id = LockDataId(tab_fd, rid, LockDataType::RECORD);
    auto& rwlock = lock_table_[lock_data_id];
    // 检查给表加 IS 锁是否能成功
    if (lock_IS_on_table(txn, tab_fd) == false) {
        return false;
    }

    for (const auto& lock : rwlock.request_queue_) {
        assert(lock.lock_mode_ == LockMode::S || lock.lock_mode_ == LockMode::X);
        if (lock.lock_mode_ == LockMode::X) {
            if (lock.txn_id_ == txn->get_transaction_id()) {
                return true;
            } else {
                return false;
            }
        }
    }
    rwlock.request_queue_.push_back({txn->get_transaction_id(), LockMode::S});
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请行级排他锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID
 * @param {int} tab_fd 记录所在的表的fd
 */
bool LockManager::lock_exclusive_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    if (txn->get_state() == TransactionState::SHRINKING) {
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    std::scoped_lock<std::mutex> latch(latch_);

    auto lock_data_id = LockDataId(tab_fd, rid, LockDataType::RECORD);
    auto& rwlock = lock_table_[lock_data_id];
    // 检查给表加 IS 锁是否能成功
    if (lock_IX_on_table(txn, tab_fd) == false) {
        return false;
    }

    for (const auto& lock : rwlock.request_queue_) {
        assert(lock.lock_mode_ == LockMode::S || lock.lock_mode_ == LockMode::X);

        if (lock.txn_id_ != txn->get_transaction_id()) {
            return false;
        }
    }
    rwlock.request_queue_.push_back({txn->get_transaction_id(), LockMode::X});
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请表级读锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_shared_on_table(Transaction* txn, int tab_fd) {
    if (txn->get_state() == TransactionState::SHRINKING) {
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    std::scoped_lock<std::mutex> latch(latch_);
    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];

    for (const auto& lock : rwlock.request_queue_) {
        if (lock.lock_mode_ == LockMode::IX || lock.lock_mode_ == LockMode::SIX || lock.lock_mode_ == LockMode::X) {
            if (lock.txn_id_ != txn->get_transaction_id()) {
                return false;
            }
        }
    }
    rwlock.request_queue_.push_back({txn->get_transaction_id(), LockMode::S});
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请表级写锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_exclusive_on_table(Transaction* txn, int tab_fd) {
    if (txn->get_state() == TransactionState::SHRINKING) {
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    std::scoped_lock<std::mutex> latch(latch_);
    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];

    for (const auto& lock : rwlock.request_queue_) {
        if (lock.txn_id_ != txn->get_transaction_id()) {
            return false;
        }
    }

    rwlock.request_queue_.push_back({txn->get_transaction_id(), LockMode::X});
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请表级意向读锁，应当在调用前加锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IS_on_table(Transaction* txn, int tab_fd) {
    if (txn->get_state() == TransactionState::ABORTED) {
        return false;
    }

    if (txn->get_state() == TransactionState::SHRINKING) {
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);

    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];

    bool is_killed = false;
    bool granted = true;
    for (const auto& lock : rwlock.request_queue_) {
        if (lock.lock_mode_ == LockMode::X) {
            if (lock.txn_id_ > txn->get_transaction_id()) {
                is_killed = true;
                txn_table_[lock.txn_id_]->set_state(TransactionState::ABORTED);
            } else if (lock.txn_id_ > txn->get_transaction_id()) {
                granted = false;
            }
        }
    }

    rwlock.request_queue_.push_back({txn->get_transaction_id(), LockMode::IS});
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请表级意向写锁，应当在调用前加锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IX_on_table(Transaction* txn, int tab_fd) {
    if (txn->get_state() == TransactionState::SHRINKING) {
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);

    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];
    for (const auto& lock : rwlock.request_queue_) {
        if (lock.lock_mode_ == LockMode::S || lock.lock_mode_ == LockMode::SIX || lock.lock_mode_ == LockMode::X) {
            if (lock.txn_id_ != txn->get_transaction_id()) {
                return false;
            }
        }
    }
    rwlock.request_queue_.push_back({txn->get_transaction_id(), LockMode::IX});
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

bool LockManager::lock_SIX_on_table(Transaction* txn, int tab_fd) {
    if (txn->get_state() == TransactionState::SHRINKING) {
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    std::scoped_lock<std::mutex> latch(latch_);
    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];

    for (const auto& lock : rwlock.request_queue_) {
        if (lock.lock_mode_ == LockMode::IX || lock.lock_mode_ == LockMode::S || lock.lock_mode_ == LockMode::SIX ||
            lock.lock_mode_ == LockMode::X) {
            if (lock.txn_id_ != txn->get_transaction_id()) {
                return false;
            }
        }
    }
    rwlock.request_queue_.push_back({txn->get_transaction_id(), LockMode::SIX});
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 释放锁
 * @return {bool} 返回解锁是否成功
 * @param {Transaction*} txn 要释放锁的事务对象指针
 * @param {LockDataId} lock_data_id 要释放的锁ID
 */
bool LockManager::unlock(Transaction* txn, LockDataId lock_data_id) {
    if (txn->get_state() == TransactionState::GROWING) {
        txn->set_state(TransactionState::SHRINKING);
    }

    std::scoped_lock<std::mutex> latch(latch_);
    auto& rwlock = lock_table_[lock_data_id];

    for (auto iter = rwlock.request_queue_.begin(); iter != rwlock.request_queue_.end();) {
        if (iter->txn_id_ == txn->get_transaction_id()) {
            iter = rwlock.request_queue_.erase(iter);
        } else {
            iter++;
        }
    }
    if (rwlock.request_queue_.empty()) {
        lock_table_.erase(lock_data_id);
    }

    return true;
}
