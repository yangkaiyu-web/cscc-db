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
 */
bool LockManager::lock_shared_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    if(txn->get_state() == TransactionState::ABORTED){
        return false;
    }
    if(txn->get_state() == TransactionState::SHRINKING){
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(),AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    auto lock = std::scoped_lock<std::mutex>(latch_);
    auto lock_data_id = LockDataId(tab_fd, rid, LockDataType::RECORD);
    auto& rwlock = lock_table_[lock_data_id];
    if (rwlock.num >= 0) {
        // refresh it and update the first_hold_shared
        assert((rwlock.num == 0 && rwlock.group_lock_mode_ == GroupLockMode::NON_LOCK && rwlock.curr == -1) ||
               (rwlock.num > 0 && rwlock.group_lock_mode_ == GroupLockMode::S));
        if (lock_IS_on_table(txn, tab_fd) == false) return false;
        rwlock.num++;
        rwlock.group_lock_mode_ = GroupLockMode::S;
        rwlock.curr = txn->get_transaction_id();
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    }
    return false;
}

/**
 * @description: 申请行级排他锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID
 * @param {int} tab_fd 记录所在的表的fd
 */
bool LockManager::lock_exclusive_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    if(txn->get_state() == TransactionState::SHRINKING){
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(),AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    auto lock = std::scoped_lock<std::mutex>(latch_);

    auto lock_data_id = LockDataId(tab_fd, rid, LockDataType::RECORD);
    auto& rwlock = lock_table_[lock_data_id];
    if (rwlock.group_lock_mode_ == GroupLockMode::NON_LOCK) {
        assert(rwlock.num == 0);
        // lock_IX_on_table必须放在这里
        if (lock_IX_on_table(txn, tab_fd) == false) return false;
        rwlock.num = -1;
        rwlock.group_lock_mode_ = GroupLockMode::X;
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    } else if (rwlock.group_lock_mode_ == GroupLockMode::S && rwlock.curr == txn->get_transaction_id() &&
               rwlock.num == 1) {
        if (lock_IX_on_table(txn, tab_fd) == false) return false;
        rwlock.num = -1;
        rwlock.group_lock_mode_ = GroupLockMode::X;
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    } else if (rwlock.group_lock_mode_ == GroupLockMode::X && rwlock.curr == txn->get_transaction_id()) {
        // 重复加锁
        assert(false);
    }

    return false;
}

/**
 * @description: 申请表级读锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_shared_on_table(Transaction* txn, int tab_fd) {
    if(txn->get_state() == TransactionState::SHRINKING){
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(),AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    auto lock = std::scoped_lock<std::mutex>(latch_);
    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];
    // if not the exclusive lock
    if (rwlock.num >= 0) {
        if (rwlock.group_lock_mode_ == GroupLockMode::NON_LOCK) {
            assert(rwlock.num == 0 && rwlock.curr == -1);
            rwlock.num = 1;
            rwlock.curr = txn->get_transaction_id();
            rwlock.group_lock_mode_ = GroupLockMode::IS;
            txn->get_lock_set()->insert(lock_data_id);
            return true;}
        else if(rwlock.group_lock_mode_==GroupLockMode::S || rwlock.group_lock_mode_==GroupLockMode::IS  ){
            // refresh it and update the first_hold_shared
        assert(rwlock.num > 0 && rwlock.curr != -1);
            rwlock.num++;
            rwlock.group_lock_mode_ = GroupLockMode::S;
            rwlock.curr = txn->get_transaction_id();
            txn->get_lock_set()->insert(lock_data_id);
            return true;
        }else if(rwlock.group_lock_mode_ == GroupLockMode::X)
    }
    return false;
}

/**
 * @description: 申请表级写锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_exclusive_on_table(Transaction* txn, int tab_fd) {
    if(txn->get_state() == TransactionState::SHRINKING){
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(),AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    auto lock = std::scoped_lock<std::mutex>{latch_};
    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];
    if ((rwlock.group_lock_mode_ == GroupLockMode::NON_LOCK) ||
        (rwlock.curr == txn->get_transaction_id() && rwlock.num == 1 &&
         (rwlock.group_lock_mode_ == GroupLockMode::S || rwlock.group_lock_mode_ == GroupLockMode::IS ||
          rwlock.group_lock_mode_ == GroupLockMode::IX))) {
        if (rwlock.group_lock_mode_ == GroupLockMode::NON_LOCK) {
            rwlock.curr = txn->get_transaction_id();
            assert(rwlock.num == 0);
        }
        rwlock.num = -1;
        rwlock.group_lock_mode_ = GroupLockMode::X;
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    }
    return false;
}

/**
 * @description: 申请表级意向读锁，应当在调用前加锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IS_on_table(Transaction* txn, int tab_fd) {
    if(txn->get_state() == TransactionState::SHRINKING){
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(),AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];
    if (rwlock.group_lock_mode_ == GroupLockMode::NON_LOCK) {
        assert(rwlock.num == 0 && rwlock.curr == -1);
        rwlock.num = 1;
        rwlock.curr = txn->get_transaction_id();
        rwlock.group_lock_mode_ = GroupLockMode::IS;
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    } else if (rwlock.group_lock_mode_ == GroupLockMode::S || rwlock.group_lock_mode_ == GroupLockMode::IS ||
               rwlock.group_lock_mode_ == GroupLockMode::IX) {
        assert(rwlock.num > 0 && rwlock.curr != -1);
        rwlock.num++;
        rwlock.curr = txn->get_transaction_id();
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    }
    return false;
}

/**
 * @description: 申请表级意向写锁，应当在调用前加锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IX_on_table(Transaction* txn, int tab_fd) {
    if(txn->get_state() == TransactionState::SHRINKING){
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(),AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];
    if (rwlock.group_lock_mode_ == GroupLockMode::NON_LOCK) {
        // assert(rwlock.num == 0 && rwlock.first == -1);
        assert(rwlock.num == 0);
        rwlock.num = 1;
        rwlock.group_lock_mode_ = GroupLockMode::IX;
        rwlock.curr = txn->get_transaction_id();
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    } else if (rwlock.group_lock_mode_ == GroupLockMode::IS || rwlock.group_lock_mode_ == GroupLockMode::IX) {
        // assert(rwlock.num > 0 && rwlock.first != -1);
        assert(rwlock.num > 0);
        rwlock.num++;
        rwlock.group_lock_mode_ = GroupLockMode::IX;
        rwlock.curr = txn->get_transaction_id();
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    } else if (rwlock.group_lock_mode_ == GroupLockMode::S && rwlock.curr == txn->get_transaction_id() &&
               rwlock.num == 1) {
        rwlock.group_lock_mode_ = GroupLockMode::SIX;
        return true;
    }
    return false;
}

bool LockManager::lock_SIX_on_table(Transaction* txn, int tab_fd) {
    if(txn->get_state() == TransactionState::SHRINKING){
        txn->set_state(TransactionState::ABORTED);
        throw TransactionAbortException(txn->get_transaction_id(),AbortReason::LOCK_ON_SHIRINKING);
    }
    txn->set_state(TransactionState::GROWING);
    auto lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    auto& rwlock = lock_table_[lock_data_id];
    if (rwlock.group_lock_mode_ == GroupLockMode::NON_LOCK) {
        // assert(rwlock.num == 0 && rwlock.first == -1);
        assert(rwlock.num == 0);
        rwlock.num = 1;
        rwlock.group_lock_mode_ = GroupLockMode::SIX;
        rwlock.curr = txn->get_transaction_id();
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    } else if (rwlock.group_lock_mode_ == GroupLockMode::IS || rwlock.group_lock_mode_ == GroupLockMode::IX) {
        // assert(rwlock.num > 0 && rwlock.first != -1);
        assert(rwlock.num > 0);
        rwlock.num++;
        rwlock.group_lock_mode_ = GroupLockMode::SIX;
        rwlock.curr = txn->get_transaction_id();
        txn->get_lock_set()->insert(lock_data_id);
        return true;
    }
    return false;
}

/**
 * @description: 释放锁
 * @return {bool} 返回解锁是否成功
 * @param {Transaction*} txn 要释放锁的事务对象指针
 * @param {LockDataId} lock_data_id 要释放的锁ID
 */
bool LockManager::unlock(Transaction* txn, LockDataId lock_data_id) {
    if(txn->get_state() == TransactionState::GROWING){
        txn->set_state(TransactionState::SHRINKING);
    }
    std::scoped_lock latch{latch_};
    auto& rwlock = lock_table_[lock_data_id];
    if (rwlock.num < 0 && rwlock.group_lock_mode_ == GroupLockMode::X) {
        rwlock.num++;
        if (rwlock.num == 0) {
            rwlock.curr = -1;
            rwlock.group_lock_mode_ = GroupLockMode::NON_LOCK;
        }
    } else {
        assert(rwlock.num > 0 &&
               (rwlock.group_lock_mode_ == GroupLockMode::S || rwlock.group_lock_mode_ == GroupLockMode::IS ||
                rwlock.group_lock_mode_ == GroupLockMode::IX));
        rwlock.num--;
        if (rwlock.num == 0) {
            rwlock.curr = -1;
            rwlock.group_lock_mode_ = GroupLockMode::NON_LOCK;
        }
    }

    return true;
}
