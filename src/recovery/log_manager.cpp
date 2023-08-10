/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "log_manager.h"

#include <cstring>
#include <memory>
#include "transaction/transaction.h"

/**
 * @description: 添加日志记录到日志缓冲区中，并返回日志记录号
 * @param {LogRecord*} log_record 要写入缓冲区的日志记录
 * @return {lsn_t} 返回该日志的日志记录号
 */
lsn_t LogManager::add_log_to_buffer(LogRecord* log_record) {
    latch_.lock();

    if (log_buffer_.is_full(log_record->log_tot_len_)) {
        flush_log_to_disk();
    } 
    log_record->serialize(log_buffer_.buffer_ + log_buffer_.offset_);
    log_buffer_.offset_ += log_record->log_tot_len_;
    buffer_lsn_ = log_record->lsn_;
    latch_.unlock();
    return log_record->lsn_;
}

/**
 * @description:
 * 把日志缓冲区的内容刷到磁盘中，由于目前只设置了一个缓冲区，因此需要阻塞其他日志操作
 */
void LogManager::flush_log_to_disk() {
    latch_.lock();
    disk_manager_->write_log(log_buffer_.buffer_, log_buffer_.offset_);
    log_buffer_.offset_=0;
    latch_.unlock();
}
void LogManager::gen_logs_from_write_set(Transaction* txn){
    for(auto& write_rec :*(txn->get_write_set()) ){
        std::unique_ptr<LogRecord>  log ;
        switch (write_rec->GetWriteType()) {
            case WType::INSERT_TUPLE:
                log = std::make_unique< InsertLogRecord>(txn->get_transaction_id(),write_rec->GetRecord(),write_rec->GetRid(),write_rec->GetTableName());
                break;                                                        
            case WType::DELETE_TUPLE:                                        

                log =  std::make_unique<DeleteLogRecord>(txn->get_transaction_id(),write_rec->GetRecord(),write_rec->GetRid(),write_rec->GetTableName());
                break;                                                        
            case WType::UPDATE_TUPLE:                                         
                log = std::make_unique< UpdateLogRecord>(txn->get_transaction_id(),write_rec->GetRecord(),write_rec->GetNewRecord(),write_rec->GetRid(),write_rec->GetTableName());
                break;
            default:
                assert(false);
        }
        log->lsn_=alloc_lsn(); // 虽然是 atomic 但还是要加锁
        log->prev_lsn_ = txn->get_so_far_lsn();
        txn->set_so_far_lsn( add_log_to_buffer(log.get()));
    }


}

void LogManager::gen_log_bein(Transaction* txn){
    auto log_record =
        std::make_unique< BeginLogRecord>(alloc_lsn(), txn->get_transaction_id(), txn->get_so_far_lsn());
    txn->set_so_far_lsn(log_record->lsn_);

    add_log_to_buffer(log_record.get());
}

void LogManager::gen_log_commit(Transaction* txn){
    auto log_record =
        std::make_unique< CommitLogRecord>(alloc_lsn(), txn->get_transaction_id(), txn->get_so_far_lsn());
    txn->set_so_far_lsn(log_record->lsn_);
    add_log_to_buffer(log_record.get());

}

void LogManager::gen_log_abort(Transaction* txn){
    auto log_record =
        std::make_unique< AbortLogRecord>(alloc_lsn(), txn->get_transaction_id(), txn->get_so_far_lsn());
    txn->set_so_far_lsn(log_record->lsn_);
    add_log_to_buffer(log_record.get());

}

