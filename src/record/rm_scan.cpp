/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "rm_scan.h"

#include "rm_file_handle.h"

/**
 * @brief 初始化file_handle和rid
 * @param file_handle
 */
RmScan::RmScan(RmFileHandle *file_handle) : file_handle_(file_handle) {
    // Todo:
    // 初始化file_handle和rid（指向第一个存放了记录的位置）
    file_handle_->RLatch();
    for (int i = 1; i < file_handle->file_hdr_.num_pages; ++i) {
        RmPageHandle page_handle = file_handle->fetch_page_handle(i);
        for (int j = 0; j < file_handle->file_hdr_.num_records_per_page; ++j) {
            if (Bitmap::is_set(page_handle.bitmap, j)) {
                rid_.page_no = i;
                rid_.slot_no = j;
                file_handle_->buffer_pool_manager_->unpin_page(
                    page_handle.page->get_page_id(), false);
                return;
            }
        }
        file_handle_->buffer_pool_manager_->unpin_page(
            page_handle.page->get_page_id(), false);
    }
    file_handle_->RUnLatch();
    rid_.page_no = INVALID_PAGE_ID;
    rid_.slot_no = -1;
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    // Todo:
    // 找到文件中下一个存放了记录的非空闲位置，用rid_来指向这个位置
    // TODO: use 2pl to optimize this latch
    file_handle_->RLatch();
    for (int i = rid_.page_no; i < file_handle_->file_hdr_.num_pages; ++i) {
        RmPageHandle page_handle = file_handle_->fetch_page_handle(i);
        for (int j = (i == rid_.page_no ? rid_.slot_no + 1 : 0);
             j < file_handle_->file_hdr_.num_records_per_page; ++j) {
            if (Bitmap::is_set(page_handle.bitmap, j)) {
                rid_.page_no = i;
                rid_.slot_no = j;
                file_handle_->buffer_pool_manager_->unpin_page(
                    page_handle.page->get_page_id(), false);
                return;
            }
        }
        file_handle_->buffer_pool_manager_->unpin_page(
            page_handle.page->get_page_id(), false);
    }
    file_handle_->RUnLatch();
    rid_.page_no = INVALID_PAGE_ID;
    rid_.slot_no = -1;
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const {
    // Todo: 修改返回值
    /*
    for (int i = rid_.page_no; i < file_handle_->file_hdr_.num_pages; ++i) {
        RmPageHandle page_handle = file_handle_->fetch_page_handle(i);
        for (int j = (i == rid_.page_no ? rid_.slot_no + 1 : 0);
             j < file_handle_->file_hdr_.num_records_per_page; ++j) {
            if (Bitmap::is_set(page_handle.bitmap, j)) {
                return false;
            }
        }
    }
    return true;
    */
    if (rid_.page_no == INVALID_PAGE_ID && rid_.slot_no == -1) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief RmScan内部存放的rid
 */
Rid RmScan::rid() const { return rid_; }
