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
#include <readline/readline.h>
#include <utility>
#include "recovery/log_defs.h"
#include "recovery/log_manager.h"

/**
 * @description: analyze阶段，需要获得脏页表（DPT）和未完成的事务列表（ATT）
 */
void RecoveryManager::analyze() {
    int offset = 0;
    char log_hdr[LOG_HEADER_SIZE];
    while(int ret = disk_manager_->read_log(log_hdr,LOG_HEADER_SIZE , offset)> 0){
        assert(ret >=0);
        offset_list_.push_back(offset);
        LogRecord  log;
        log.deserialize(log_hdr);
        lsn_offset_table_.insert(std::make_pair(log.lsn_, offset));
        offset += log.log_tot_len_;
    }

}

/**
 * @description: 重做所有未落盘的操作
 */
void RecoveryManager::redo() {
    int offset = 0;
    char log_hdr[LOG_HEADER_SIZE];
    while(disk_manager_->read_log(log_hdr,LOG_HEADER_SIZE , offset)> 0){
        LogRecord  log;
        log.deserialize(log_hdr);
        lsn_offset_table_.insert(std::make_pair(log.lsn_, offset));
        char * log_buf = new char[log.log_tot_len_];
        disk_manager_->read_log(log_buf, log.log_tot_len_, offset);
        offset += log.log_tot_len_;
        if(log.log_type_ == LogType:: DELETE || log.log_type_ == LogType:: CLR_DELETE){
            DeleteLogRecord del_log ;
            del_log.deserialize(log_buf);
            std::string tab_name(del_log.table_name_,del_log.table_name_size_);
            auto fh = sm_manager_->fhs_.at(tab_name).get();
            fh->delete_record(del_log.rid_,nullptr );
        }else if(log.log_type_ == LogType::INSERT || log.log_type_ == LogType:: CLR_INSERT){
                InsertLogRecord insert_log ;
                insert_log.deserialize(log_buf);
                std::string tab_name(insert_log.table_name_,insert_log.table_name_size_);
                auto fh = sm_manager_->fhs_.at(tab_name).get();
                fh->insert_record(insert_log.rid_,insert_log.insert_value_.data,);
        } else if(log.log_type_ == LogType::UPDATE || log.log_type_ == LogType :: CLR_UPDATE){
                UpdateLogRecord update_log ;
                update_log.deserialize(log_buf);
                std::string tab_name(update_log.table_name_,update_log.table_name_size_);
                auto fh = sm_manager_->fhs_.at(tab_name).get();
                fh->insert_record(update_log.rid_,update_log.new_value_.data);
        }else if(log.log_type_ == LogType::BEGIN){
             undo_list_.insert( log.log_tid_);
        }else if(log.log_type_ == LogType::COMMIT || log.log_type_ == LogType::ABORT){
            assert(undo_list_.find(log.log_tid_)!=undo_list_.end());
             undo_list_.erase( log.log_tid_);
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

    * /

    for (auto offset_it = offset_list_.rbegin(); offset_it != offset_list_.rend(); ++offset_it) {
        int offset = *offset_it;
        char log_hdr[LOG_HEADER_SIZE];
        int ret = disk_manager_->read_log(log_hdr,LOG_HEADER_SIZE , offset);
        assert(ret > 0);


        LogRecord  log;
        log.deserialize(log_hdr);
        char * log_buf = new char[log.log_tot_len_];
        disk_manager_->read_log(log_buf, log.log_tot_len_, offset);

        if(log.log_type_ == LogType:: DELETE ){
            DeleteLogRecord del_log ;
            del_log.deserialize(log_buf);
            std::string tab_name(del_log.table_name_,del_log.table_name_size_);
            auto fh = sm_manager_->fhs_.at(tab_name).get();
            fh->delete_record(del_log.rid_,nullptr );

        }else if(log.log_type_ == LogType::INSERT || log.log_type_ == LogType:: CLR_INSERT){
                InsertLogRecord insert_log ;
                insert_log.deserialize(log_buf);
                std::string tab_name(insert_log.table_name_,insert_log.table_name_size_);
                auto fh = sm_manager_->fhs_.at(tab_name).get();
                fh->insert_record(insert_log.rid_,insert_log.insert_value_.data);
        } else if(log.log_type_ == LogType::UPDATE || log.log_type_ == LogType :: CLR_UPDATE){
                UpdateLogRecord update_log ;
                update_log.deserialize(log_buf);
                std::string tab_name(update_log.table_name_,update_log.table_name_size_);
                auto fh = sm_manager_->fhs_.at(tab_name).get();
                fh->insert_record(update_log.rid_,update_log.new_value_.data);
        }else if(log.log_type_ == LogType::BEGIN){

             undo_list_.erase( log.log_tid_);
        }else if(log.log_type_ == LogType::COMMIT || log.log_type_ == LogType::ABORT){
            assert(false);
        }
    }


}
