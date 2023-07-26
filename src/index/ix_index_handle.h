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

#include "ix_defs.h"
#include "transaction/transaction.h"

enum class Operation { FIND = 0, INSERT, DELETE };  // 三种操作：查找、插入、删除

static const bool binary_search = false;

inline int ix_compare(const char *a, const char *b, ColType type, int col_len) {
    switch (type) {
        case TYPE_INT: {
            int ia = *(int *)a;
            int ib = *(int *)b;
            return (ia < ib) ? -1 : ((ia > ib) ? 1 : 0);
        }
        case TYPE_FLOAT: {
            float fa = *(float *)a;
            float fb = *(float *)b;
            return (fa < fb) ? -1 : ((fa > fb) ? 1 : 0);
        }
        case TYPE_STRING:
            return memcmp(a, b, col_len);
        default:
            throw InternalError("Unexpected data type");
    }
}

inline int ix_compare(const char *a, const char *b, const std::vector<ColType> &col_types,
                      const std::vector<int> &col_lens, size_t pre = 0) {
    pre = (pre > 0 ? pre : col_lens.size());
    int offset = 0;
    for (size_t i = 0; i < pre; ++i) {
        int res = ix_compare(a + offset, b + offset, col_types[i], col_lens[i]);
        if (res != 0) return res;
        offset += col_lens[i];
    }
    return 0;
}

/* 管理B+树中的每个节点 */
class IxNodeHandle {
    friend class IxIndexHandle;
    friend class IxScan;

   private:
    const IxFileHdr *file_hdr;  // 节点所在文件的头部信息
    Page *page;                 // 存储节点的页面
    IxPageHdr *page_hdr;        // page->data的第一部分，指针指向首地址，长度为sizeof(IxPageHdr)
    char *keys;  // page->data的第二部分，指针指向首地址，长度为file_hdr->keys_size，每个key的长度为file_hdr->col_len
    Rid *rids;   // page->data的第三部分，指针指向首地址

   public:
    IxNodeHandle() = default;

    IxNodeHandle(const IxFileHdr *file_hdr_, Page *page_) : file_hdr(file_hdr_), page(page_) {
        page_hdr = reinterpret_cast<IxPageHdr *>(page->get_data());
        keys = page->get_data() + sizeof(IxPageHdr);
        rids = reinterpret_cast<Rid *>(keys + file_hdr->keys_size_);
    }

    int get_size() { return page_hdr->num_key; }

    void set_size(int size) { page_hdr->num_key = size; }

    // rid数量必须满足小于等于
    inline int get_max_rid_size() { return file_hdr->btree_order_; }

    // rid数量必须满足大于等于
    inline int get_min_rid_size() { return (get_max_rid_size() + 1) >> 1; }

    // key数量必须满足小于等于
    inline int get_max_key_size() { return is_leaf_page() ? get_max_rid_size() : get_max_rid_size() - 1; }

    inline int get_min_key_size() { return is_leaf_page() ? get_min_rid_size() : get_min_rid_size() - 1; }

    // int key_at(int i) { return *(int *)get_key(i); }

    /* 得到第i个孩子结点的page_no */
    page_id_t value_at(int i) { return get_rid(i)->page_no; }

    page_id_t get_page_no() { return page->get_page_id().page_no; }

    PageId get_page_id() { return page->get_page_id(); }

    page_id_t get_next_leaf() { return page_hdr->next_leaf; }

    page_id_t get_prev_leaf() { return page_hdr->prev_leaf; }

    page_id_t get_parent_page_no() { return page_hdr->parent; }

    bool is_leaf_page() { return page_hdr->is_leaf; }

    bool is_root_page() { return get_parent_page_no() == IX_NO_PAGE; }

    void set_next_leaf(page_id_t page_no) { page_hdr->next_leaf = page_no; }

    void set_prev_leaf(page_id_t page_no) { page_hdr->prev_leaf = page_no; }

    void set_parent_page_no(page_id_t parent) { page_hdr->parent = parent; }

    char *get_key(int key_idx) const { return keys + key_idx * file_hdr->col_tot_len_; }

    Rid *get_rid(int rid_idx) const { return &rids[rid_idx]; }

    void set_key(int key_idx, const char *key) {
        memcpy(keys + key_idx * file_hdr->col_tot_len_, key, file_hdr->col_tot_len_);
    }

    void set_rid(int rid_idx, const Rid &rid) { rids[rid_idx] = rid; }

    int lower_bound(const char *target, size_t pre = 0) const;

    int upper_bound(const char *target, size_t pre = 0) const;

    // YKY：感觉不会用到
    // void insert_pairs(int pos, const char *key, const Rid *rid, int n);

    page_id_t internal_lookup(const char *key, size_t pre = 0);

    bool leaf_lookup(const char *key, Rid **value);

    int insert(const char *key, const Rid &value);

    // 仅用于在叶子结点中的指定位置插入单个键值对
    void insert_pair(int pos, const char *key, const Rid &rid) {
        assert(pos <= page_hdr->num_key);
        for (int i = page_hdr->num_key; i > pos; --i) {
            memcpy(get_key(i), get_key(i - 1), file_hdr->col_tot_len_);
            *get_rid(i) = *get_rid(i - 1);
        }
        memcpy(get_key(pos), key, file_hdr->col_tot_len_);
        *get_rid(pos) = rid;
        ++page_hdr->num_key;
    }

    void erase_pair(int pos);

    int remove(const char *key);

    inline void RLatch() { page->RLatch(); }
    inline void RUnLatch() { page->RUnLatch(); }
    inline void WLatch() { page->WLatch(); }
    inline void WUnLatch() { page->WUnLatch(); }

    /**
     * @brief used in internal node to remove the last key in root node, and
     * return the last child
     *
     * @return the last child
     */
    page_id_t remove_and_return_only_child() {
        assert(get_size() == 1);
        page_id_t child_page_no = value_at(0);
        erase_pair(0);
        assert(get_size() == 0);
        return child_page_no;
    }

    /**
     * @brief
     * 由parent调用，寻找child，返回child在parent中的rid_idx∈[0,page_hdr->num_key)
     * @param child
     * @return int
     */
    int find_child(IxNodeHandle *child) {
        int rid_idx;
        for (rid_idx = 0; rid_idx <= page_hdr->num_key; rid_idx++) {
            if (get_rid(rid_idx)->page_no == child->get_page_no()) {
                break;
            }
        }
        assert(rid_idx <= page_hdr->num_key);
        return rid_idx;
    }
};

/* B+树 */
class IxIndexHandle {
    friend class IxScan;
    friend class IxManager;

   private:
    DiskManager *disk_manager_;
    BufferPoolManager *buffer_pool_manager_;
    int fd_;                        // 存储B+树的文件
    IxFileHdr *file_hdr_;           // 存了root_page，但其初始化为1（第0页存FILE_HDR_PAGE）
    std::shared_mutex root_latch_;  // 保护file_hdr_

   public:
    IxIndexHandle(DiskManager *disk_manager, BufferPoolManager *buffer_pool_manager, int fd);

    // for search
    bool is_exist(const char *key, Transaction *transaction);

    bool get_value(const char *key, std::vector<Rid> &result, Transaction *transaction);

    IxNodeHandle *find_leaf_page(const char *key, Operation operation, Transaction *transaction,
                                 bool find_first = false);

    // for insert
    page_id_t insert_entry(const char *key, const Rid &value, Transaction *transaction);

    IxNodeHandle *split(IxNodeHandle *node);

    void insert_into_parent(IxNodeHandle *old_node, const char *key, IxNodeHandle *new_node, Transaction *transaction);

    // for delete
    bool delete_entry(const char *key, Transaction *transaction);

    bool coalesce_or_redistribute(IxNodeHandle *node, Transaction *transaction = nullptr,
                                  bool *root_is_latched = nullptr);
    bool adjust_root(IxNodeHandle *old_root_node);

    // TRY:不需要parent参数，函数内部可以获得
    // void redistribute(IxNodeHandle *neighbor_node, IxNodeHandle *node,
    //                  IxNodeHandle *parent, int index);
    void redistribute(IxNodeHandle *neighbor_node, IxNodeHandle *node);

    void coalesce(IxNodeHandle *left_node, IxNodeHandle *right_node, Transaction *transaction = nullptr,
                  bool *root_is_latched = nullptr);

    // 删除右节点页，更新右节点的父节点的kv，删去其中和右节点相关的kv，可能需要进一步更新祖先节点
    void update_right_parent(IxNodeHandle *right_node);

    Iid lower_bound(const char *key, size_t pre = 0);

    Iid upper_bound(const char *key, size_t pre = 0);

    Iid leaf_end() const;

    Iid leaf_begin() const;

   private:
    // 辅助函数
    // @brief:找到node的所有叶子节点中最小key值所在的节点，记得在外面unpin返回的leaf_node，因此不能直接返回char*
    IxNodeHandle *get_min_leafnode(IxNodeHandle *node) {
        if (node->is_leaf_page()) {
            return node;
        }

        page_id_t page_id = node->get_rid(0)->page_no;
        while (true) {
            node = fetch_node(page_id);
            if (node->is_leaf_page()) {
                return node;
            }
            page_id = node->get_rid(0)->page_no;
            release_node_handle(node, false);
        }
    }

    bool is_empty() const { return file_hdr_->root_page_ == IX_NO_PAGE; }

    // for get/create node
    IxNodeHandle *fetch_node(int page_no) const;

    IxNodeHandle *create_node();

    // for maintain data structure
    void maintain_parent(IxNodeHandle *node);

    void erase_leaf(IxNodeHandle *leaf);

    void release_node_handle(IxNodeHandle *node, bool is_dirty) const;

    void maintain_child(IxNodeHandle *node, int child_idx);

    // for index test
    Rid get_rid(const Iid &iid) const;
};