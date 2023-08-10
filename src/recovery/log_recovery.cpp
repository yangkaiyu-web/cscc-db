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

/**
 * @description: analyze阶段，需要获得脏页表（DPT）和未完成的事务列表（ATT）
 */
void RecoveryManager::analyze() {
 
}

/**
 * @description: 重做所有未落盘的操作
 */
void RecoveryManager::redo() {

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
}
