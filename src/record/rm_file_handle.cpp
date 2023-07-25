/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "rm_file_handle.h"

/**
 * @description: 获取当前表中记录号为rid的记录
 * @param {Rid&} rid 记录号，指定记录的位置
 * @param {Context*} context
 * @return {unique_ptr<RmRecord>} rid对应的记录对象指针
 */
std::unique_ptr<RmRecord> RmFileHandle::get_record(const Rid &rid,
                                                   Context *context) const {
    // Todo:
    // 1. 获取指定记录所在的page handle
    // 2. 初始化一个指向RmRecord的指针（赋值其内部的data和size）
    RmPageHandle page_handle = fetch_page_handle(rid.page_no);
    int record_size = page_handle.file_hdr->record_size;
    char *data = page_handle.get_slot(rid.slot_no);
    auto res = std::make_unique<RmRecord>(record_size, data);
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    return res;
}

/**
 * @description: 在当前表中插入一条记录，不指定插入位置
 * @param {char*} buf 要插入的记录的数据
 * @param {Context*} context
 * @return {Rid} 插入的记录的记录号（位置）
 */
Rid RmFileHandle::insert_record(char *buf, Context *context) {
    // Todo:
    // 1. 获取当前未满的page handle
    // 2. 在page handle中找到空闲slot位置
    // 3. 将buf复制到空闲slot位置
    // 4. 更新page_handle.page_hdr中的数据结构
    // 注意考虑插入一条记录后页面已满的情况，需要更新file_hdr_.first_free_page_no
    RmPageHandle page_hdl = create_page_handle();
    int i;
    for (i = 0; i < file_hdr_.num_records_per_page; ++i) {
        if (!Bitmap::is_set(page_hdl.bitmap, i)) {
            memcpy(page_hdl.get_slot(i), buf, file_hdr_.record_size);
            Bitmap::set(page_hdl.bitmap, i);
            auto &page_hdr = page_hdl.page_hdr;
            ++page_hdr->num_records;
            if (page_hdr->num_records == file_hdr_.num_records_per_page) {
                file_hdr_.first_free_page_no = page_hdr->next_free_page_no;
            }
            const PageId &page_id = page_hdl.page->get_page_id();
            buffer_pool_manager_->unpin_page(page_id, true);
            return Rid{page_id.page_no, i};
        }
    }
    assert(false);
    return Rid{-1, -1};
}

/**
 * @description: 在当前表中的指定位置插入一条记录
 * @param {Rid&} rid 要插入记录的位置
 * @param {char*} buf 要插入记录的数据
 */
void RmFileHandle::insert_record(const Rid &rid, char *buf) {
    RmPageHandle page_handle = fetch_page_handle(rid.page_no);
    if (!Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
        memcpy(page_handle.get_slot(rid.slot_no), buf, file_hdr_.record_size);
        Bitmap::set(page_handle.bitmap, rid.slot_no);
        auto &page_hdr = page_handle.page_hdr;
        ++page_hdr->num_records;
        if (page_hdr->num_records == file_hdr_.num_records_per_page) {
            file_hdr_.first_free_page_no = page_hdr->next_free_page_no;
        }
    } else {
        assert(false);
    }
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), true);
}
/**
 * @description: 删除记录文件中记录号为rid的记录
 * @param {Rid&} rid 要删除的记录的记录号（位置）
 * @param {Context*} context
 */
void RmFileHandle::delete_record(const Rid &rid, Context *context) {
    // Todo:
    // 1. 获取指定记录所在的page handle
    // 2. 更新page_handle.page_hdr中的数据结构
    // 注意考虑删除一条记录后页面未满的情况，需要调用release_page_handle()
    RmPageHandle page_handle = fetch_page_handle(rid.page_no);
    if (Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
        Bitmap::reset(page_handle.bitmap, rid.slot_no);
        auto &page_hdr = page_handle.page_hdr;
        --page_hdr->num_records;
        if (page_hdr->num_records == file_hdr_.num_records_per_page - 1) {
            page_hdr->next_free_page_no = file_hdr_.first_free_page_no;
            file_hdr_.first_free_page_no = rid.page_no;
        }
    } else {
        assert(false);
    }
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), true);
}

/**
 * @description: 更新记录文件中记录号为rid的记录
 * @param {Rid&} rid 要更新的记录的记录号（位置）
 * @param {char*} buf 新记录的数据
 * @param {Context*} context
 */
void RmFileHandle::update_record(const Rid &rid, char *buf, Context *context) {
    // Todo:
    // 1. 获取指定记录所在的page handle
    // 2. 更新记录
    RmPageHandle page_handle = fetch_page_handle(rid.page_no);
    if (Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
        memcpy(page_handle.get_slot(rid.slot_no), buf, file_hdr_.record_size);
    } else {
        assert(false);
    }
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), true);
}

/**
 * 以下函数为辅助函数，仅提供参考，可以选择完成如下函数，也可以删除如下函数，在单元测试中不涉及如下函数接口的直接调用
 */
/**
 * @description: 获取指定页面的页面句柄
 * @param {int} page_no 页面号
 * @return {RmPageHandle} 指定页面的句柄
 */
RmPageHandle RmFileHandle::fetch_page_handle(int page_no) const {
    // Todo:
    // 使用缓冲池获取指定页面，并生成page_handle返回给上层
    // if page_no is invalid, throw PageNotExistError exception
    Page *page = buffer_pool_manager_->fetch_page({fd_, page_no});
    return RmPageHandle(&file_hdr_, page);
}

/**
 * @description: 创建一个新的page handle
 * @return {RmPageHandle} 新的PageHandle
 */
RmPageHandle RmFileHandle::create_new_page_handle() {
    // Todo:
    // 1.使用缓冲池来创建一个新page
    // 2.更新page handle中的相关信息
    // 3.更新file_hdr_
    PageId page_id{fd_, INVALID_PAGE_ID};
    Page *page = buffer_pool_manager_->new_page(&page_id);

    assert(page != nullptr);
    RmPageHandle page_hdl(&file_hdr_, page);
    page_hdl.page_hdr->next_free_page_no = -1;
    page_hdl.page_hdr->num_records = 0;
    file_hdr_.num_pages++;
    return page_hdl;
}

/**
 * @brief 创建或获取一个空闲的page handle，更新first_free_page_no
 *
 * @return RmPageHandle 返回生成的空闲page handle
 * @note pin the page, remember to unpin it outside!
 */
RmPageHandle RmFileHandle::create_page_handle() {
    // Todo:
    // 1. 判断file_hdr_中是否还有空闲页
    //     1.1
    //     没有空闲页：使用缓冲池来创建一个新page；可直接调用create_new_page_handle()
    //     1.2 有空闲页：直接获取第一个空闲页
    // 2. 生成page handle并返回给上层
    int free_page_no = file_hdr_.first_free_page_no;

    if (free_page_no == -1) {
        PageId page_id{fd_, INVALID_PAGE_ID};
        Page *page = buffer_pool_manager_->new_page(&page_id);
        assert(page != nullptr);
        RmPageHandle page_hdl(&file_hdr_, page);
        page_hdl.page_hdr->next_free_page_no = -1;
        page_hdl.page_hdr->num_records = 0;
        file_hdr_.num_pages++;
        file_hdr_.first_free_page_no = page_id.page_no;
        return page_hdl;
    }

    Page *page = buffer_pool_manager_->fetch_page({fd_, free_page_no});
    assert(page != nullptr);
    RmPageHandle page_hdl(&file_hdr_, page);
    return page_hdl;
}

/**
 * @description:
 * 当一个页面从没有空闲空间的状态变为有空闲空间状态时，更新文件头和页头中空闲页面相关的元数据
 */
void RmFileHandle::release_page_handle(RmPageHandle &page_handle) {
    // Todo:
    // 当page从已满变成未满，考虑如何更新：
    // 1. page_handle.page_hdr->next_free_page_no
    // 2. file_hdr_.first_free_page_no
}