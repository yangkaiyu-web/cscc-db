/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include <assert.h>

#include <memory>

#include "bitmap.h"
#include "common/context.h"
#include "recovery/log_manager.h"
#include "rm_defs.h"
#include "transaction/transaction.h"

class RmManager;

/* 对表数据文件中的页面进行封装 */
struct RmPageHandle {
    const RmFileHdr *file_hdr;  // 当前页面所在文件的文件头指针
    Page *page;                 // 页面的实际数据，包括页面存储的数据、元信息等
    RmPageHdr *page_hdr;  // page->data的第一部分，存储页面元信息，指针指向首地址，长度为sizeof(RmPageHdr)
    char *bitmap;  // page->data的第二部分，存储页面的bitmap，指针指向首地址，长度为file_hdr->bitmap_size
    char *slots;  // page->data的第三部分，存储表的记录，指针指向首地址，每个slot的长度为file_hdr->record_size
    RmPageHandle() = default;

    RmPageHandle(const RmFileHdr *fhdr_, Page *page_) : file_hdr(fhdr_), page(page_) {
        page_hdr = reinterpret_cast<RmPageHdr *>(page->get_data() + page->OFFSET_PAGE_HDR);
        bitmap = page->get_data() + sizeof(RmPageHdr) + page->OFFSET_PAGE_HDR;
        slots = bitmap + file_hdr->bitmap_size;
    }

    // 返回指定slot_no的slot存储首地址
    char *get_slot(int slot_no) const {
        return slots + slot_no * file_hdr->record_size;  // slots的首地址 + slot个数 *
                                                         // 每个slot的大小(每个record的大小)
    }

    inline void RLatch() { page->RLatch(); }
    inline void RUnLatch() { page->RUnLatch(); }
    inline void WLatch() { page->WLatch(); }
    inline void WUnLatch() { page->WUnLatch(); }
};

/* 每个RmFileHandle对应一个表的数据文件，里面有多个page，每个page的数据封装在RmPageHandle中
 */
class RmFileHandle {
    friend class RmScan;
    friend class RmManager;

   public:
    mutable std::shared_mutex hdr_latch_;  // 保护file_hdr_中的num_pages和first_free_page_no

   private:
    DiskManager *disk_manager_;
    BufferPoolManager *buffer_pool_manager_;
    const int fd_;        // 打开文件后产生的文件句柄
    RmFileHdr file_hdr_;  // 文件头，维护当前表文件的元数据

   public:
    RmFileHandle(DiskManager *disk_manager, BufferPoolManager *buffer_pool_manager, int fd)
        : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager), fd_(fd) {
        // 注意：这里从磁盘中读出文件描述符为fd的文件的file_hdr，读到内存中
        // 这里实际就是初始化file_hdr，只不过是从磁盘中读出进行初始化
        // init file_hdr_
        disk_manager_->read_page(fd, RM_FILE_HDR_PAGE, (char *)&file_hdr_, sizeof(file_hdr_));
        // disk_manager管理的fd对应的文件中，设置从file_hdr_.num_pages开始分配page_no
        disk_manager_->set_fd2pageno(fd, file_hdr_.num_pages);
    }

    const RmFileHdr &get_file_hdr() const { return file_hdr_; }
    int GetFd() { return fd_; }

    // 无需加锁
    inline int get_record_size() const { return file_hdr_.record_size; }

    /* 判断指定位置上是否已经存在一条记录，通过Bitmap来判断 */
    bool is_record(const Rid &rid) const {
        std::shared_lock<std::shared_mutex> lock(hdr_latch_);
        RmPageHandle page_handle = fetch_page_handle(rid.page_no);
        bool res = Bitmap::is_set(page_handle.bitmap, rid.slot_no);
        buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
        return res;  // page的slot_no位置上是否有record
    }

    std::unique_ptr<RmRecord> get_record(const Rid &rid, Context *context) const;

    Rid insert_record(char *buf, Context *context);
    // used for recovery
    void insert_record(const Rid &rid, char *buf);

    void delete_record(const Rid &rid, Context *context);
    // used for recovery
    void delete_record(const Rid &rid);

    void update_record(const Rid &rid, char *buf, Context *context);

    RmPageHandle fetch_page_handle(int page_no) const;

    void unpin_page(PageId page_id, bool is_dirty) const;

    inline void RLatch() const { hdr_latch_.lock_shared(); }

    inline void RUnLatch() const { hdr_latch_.unlock_shared(); }

    inline void WLatch() const { hdr_latch_.lock(); }

    inline void WUnLatch() const { hdr_latch_.unlock(); }

    void desc_hdr() {
        std::cout << file_hdr_.record_size << std::endl;
        std::cout << file_hdr_.num_pages << std::endl;
        std::cout << file_hdr_.num_records_per_page << std::endl;
        std::cout << file_hdr_.first_free_page_no << std::endl;
        std::cout << file_hdr_.bitmap_size << std::endl;
    }

   private:
    RmPageHandle fetch_free_page_handle();
};
