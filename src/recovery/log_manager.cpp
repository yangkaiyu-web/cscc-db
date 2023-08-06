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

/**
 * @description: 添加日志记录到日志缓冲区中，并返回日志记录号
 * @param {LogRecord*} log_record 要写入缓冲区的日志记录
 * @return {lsn_t} 返回该日志的日志记录号
 */
lsn_t LogManager::add_log_to_buffer(LogRecord* log_record) {
    latch_.lock();
    if (!log_buffer_.is_full(log_record->log_tot_len_)) {
        log_record->serialize(log_buffer_.buffer_ + log_buffer_.offset_);
        log_buffer_.offset_ += log_record->log_tot_len_;
        buffer_lsn_ = log_record->lsn_;
    } else {
        assert(false);
    }
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
    persist_lsn_ = buffer_lsn_;
    latch_.unlock();
}
