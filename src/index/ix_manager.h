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

#include <memory>
#include <string>

#include "ix_defs.h"
#include "ix_index_handle.h"
#include "record/rm_file_handle.h"
#include "record/rm_scan.h"
#include "system/sm_meta.h"

class IxManager {
   private:
    DiskManager *disk_manager_;
    BufferPoolManager *buffer_pool_manager_;

   public:
    IxManager(DiskManager *disk_manager, BufferPoolManager *buffer_pool_manager)
        : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager) {}

    std::string get_index_name(const std::string &filename, const std::vector<std::string> &index_cols) {
        std::string index_name = filename;
        for (size_t i = 0; i < index_cols.size(); ++i) index_name += "_" + index_cols[i];
        index_name += ".idx";

        return index_name;
    }

    std::string get_index_name(const std::string &filename, const std::vector<ColMeta> &index_cols) {
        std::string index_name = filename;
        for (size_t i = 0; i < index_cols.size(); ++i) index_name += "_" + index_cols[i].name;
        index_name += ".idx";

        return index_name;
    }

    bool exists(const std::string &filename, const std::vector<ColMeta> &index_cols) {
        auto ix_name = get_index_name(filename, index_cols);
        return disk_manager_->is_file(ix_name);
    }

    bool exists(const std::string &filename, const std::vector<std::string> &index_cols) {
        auto ix_name = get_index_name(filename, index_cols);
        return disk_manager_->is_file(ix_name);
    }

    // TODO:应当加锁？
    void create_index(const std::string &filename, const std::vector<ColMeta> &index_cols) {
        std::string ix_name = get_index_name(filename, index_cols);
        // Create index file
        disk_manager_->create_file(ix_name);
        // Open index file
        int fd = disk_manager_->open_file(ix_name);

        // Create file header and write to file
        // Theoretically: |page_hdr| + (|attr| + |rid|) * n + |rid| <= PAGE_SIZE
        // but we reserve one slot for convenient inserting and deleting, i.e.
        // |page_hdr| + (|attr| + |rid|) * (n + 1) + |rid| <= PAGE_SIZE
        int col_tot_len = 0;
        int col_num = index_cols.size();
        for (auto &col : index_cols) {
            col_tot_len += col.len;
        }
        if (col_tot_len > IX_MAX_COL_LEN) {
            throw InvalidColLengthError(col_tot_len);
        }
        // 根据 |page_hdr| + (|attr| + |rid|) * (n + 1) <= PAGE_SIZE
        // 求得n的最大值btree_order 即 n <=
        // btree_order，那么btree_order就是每个结点最多可插入的键值对数量（实际还多留了一个空位，但其不可插入）
        /* TRY:暂时将btree order修改为常数
        int btree_order =
            static_cast<int>{[PAGE_SIZE - sizeof(IxPageHdr)] /
                                 [col_tot_len + sizeof(Rid)] -
                             1};
                             */
        int btree_order = 7;
        assert(btree_order > 2);

        // Create file header and write to file
        IxFileHdr *fhdr =
            new IxFileHdr(IX_NO_PAGE, IX_INIT_NUM_PAGES, IX_INIT_ROOT_PAGE, col_num, col_tot_len, btree_order,
                          (btree_order + 1) * col_tot_len, IX_INIT_ROOT_PAGE, IX_INIT_ROOT_PAGE);
        for (int i = 0; i < col_num; ++i) {
            fhdr->col_types_.push_back(index_cols[i].type);
            fhdr->col_lens_.push_back(index_cols[i].len);
        }
        fhdr->update_tot_len();

        char *data = new char[fhdr->tot_len_];
        fhdr->serialize(data);

        disk_manager_->write_page(fd, IX_FILE_HDR_PAGE, data, fhdr->tot_len_);
        delete[] data;

        disk_manager_->set_fd2pageno(fd, IX_INIT_ROOT_PAGE);
        // 注意root node页号为1，也标记为叶子结点，其前一个/后一个叶子均指向leaf
        // header Create root node and write to file
        {
            // 无需++file_hdr的num page，因为上面已经设置为2
            PageId new_page_id = {.fd = fd, .page_no = INVALID_PAGE_ID};
            // 从1开始分配page_no
            Page *page = buffer_pool_manager_->new_page(&new_page_id);
            assert(page->get_page_id().page_no == IX_INIT_ROOT_PAGE);
            auto phdr = reinterpret_cast<IxPageHdr *>(page->get_data());
            *phdr = {
                .next_free_page_no = IX_NO_PAGE,
                .parent = IX_NO_PAGE,
                .num_key = 0,
                .is_leaf = true,
                .prev_leaf = IX_NO_PAGE,
                .next_leaf = IX_NO_PAGE,
            };
            buffer_pool_manager_->unpin_page(page->get_page_id(), true);
            buffer_pool_manager_->flush_page(page->get_page_id());
            buffer_pool_manager_->free_page(page->get_page_id());
        }

        // Close index file
        disk_manager_->close_file(fd);
    }

    // 创建索引时初始化b+树
    void init_tree(IxIndexHandle *ih, RmFileHandle *fh, std::vector<ColMeta> &col_meta_vec) {
        std::unique_ptr<RmScan> scan_ = std::make_unique<RmScan>(fh);
        char *key = new char[ih->file_hdr_->col_tot_len_];
        for (auto rid = scan_->rid(); !scan_->is_end(); scan_->next()) {
            rid = scan_->rid();
            auto record = fh->get_record(rid, nullptr);
            int offset = 0;
            for (auto &col_meta : col_meta_vec) {
                memcpy(key + offset, record->data + col_meta.offset, col_meta.len);
                offset += col_meta.len;
            }
            assert(offset == ih->file_hdr_->col_tot_len_);
            ih->insert_entry(key, rid, nullptr);
        }
        delete[] key;
    }

    void destroy_index(const std::string &filename, const std::vector<ColMeta> &index_cols) {
        std::string ix_name = get_index_name(filename, index_cols);
        disk_manager_->destroy_file(ix_name);
    }

    void destroy_index(const std::string &filename, const std::vector<std::string> &index_cols) {
        std::string ix_name = get_index_name(filename, index_cols);
        disk_manager_->destroy_file(ix_name);
    }

    // 注意这里打开文件，创建并返回了index file handle的指针
    std::unique_ptr<IxIndexHandle> open_index(const std::string &filename, const std::vector<ColMeta> &index_cols) {
        std::string ix_name = get_index_name(filename, index_cols);
        int fd = disk_manager_->open_file(ix_name);
        return std::make_unique<IxIndexHandle>(disk_manager_, buffer_pool_manager_, fd);
    }

    std::unique_ptr<IxIndexHandle> open_index(const std::string &filename, const std::vector<std::string> &index_cols) {
        std::string ix_name = get_index_name(filename, index_cols);
        int fd = disk_manager_->open_file(ix_name);
        return std::make_unique<IxIndexHandle>(disk_manager_, buffer_pool_manager_, fd);
    }

    std::unique_ptr<IxIndexHandle> open_index(const std::string &index_name) {
        int fd = disk_manager_->open_file(index_name);
        return std::make_unique<IxIndexHandle>(disk_manager_, buffer_pool_manager_, fd);
    }

    void close_index(const IxIndexHandle *ih) {
        char *data = new char[ih->file_hdr_->tot_len_];
        ih->file_hdr_->serialize(data);
        disk_manager_->write_page(ih->fd_, IX_FILE_HDR_PAGE, data, ih->file_hdr_->tot_len_);
        delete[] data;
        // 缓冲区的所有页刷到磁盘，注意这句话必须写在close_file前面
        buffer_pool_manager_->flush_all_pages(ih->fd_);
        buffer_pool_manager_->free_all_pages(ih->fd_);
        disk_manager_->close_file(ih->fd_);
    }
};