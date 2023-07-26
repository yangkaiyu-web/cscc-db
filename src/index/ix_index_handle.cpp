/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */
#include "ix_index_handle.h"

#include "ix_scan.h"

/**
 * @brief 在当前node中查找第一个>=target的key_idx
 * key:       [0]        [1]     [2]    ...     [num_key-1]
 *        (<)/   \(>=)  /   \   /   \            /       \
 * value: [0]       [1]      [2]    [3]     ...        [num_key]
 * @return
 * key_idx，范围为[0,num_key]，如果返回的idx=num_key，则表示target大于最后一个key
 * @note 返回val/rid index，作为slot no
 */
int IxNodeHandle::lower_bound(const char *target, size_t pre) const {
    // Todo:
    // 查找当前节点中第一个大于等于target的key，并返回val/rid的位置给上层
    // 提示:
    // 可以采用多种查找方式，如顺序遍历、二分查找等；使用ix_compare()函数进行比较
    if (page_hdr->num_key == 0) return 0;
    const std::vector<ColType> &col_types = file_hdr->col_types_;
    const std::vector<int> &col_lens = file_hdr->col_lens_;
    pre = (pre == 0 ? static_cast<size_t>(file_hdr->col_num_) : pre);
    int l = 0, r = page_hdr->num_key - 1;
    while (l < r) {
        int mid = (l + r) >> 1;
        if (ix_compare(get_key(mid), target, col_types, col_lens, pre) < 0) {
            l = mid + 1;
        } else {
            r = mid;
        }
    }
    if (ix_compare(get_key(r), target, col_types, col_lens, pre) >= 0) {
        return r;
    }
    return page_hdr->num_key;
}

/**
 * @brief 在当前node中查找第一个>target的key_idx
 * key:       [0]        [1]     [2]    ...     [num_key-1]
 *        (<)/   \(>=)  /   \   /   \            /       \
 * value: [0]       [1]      [2]    [3]     ...        [num_key]
 * @return
 * key_idx，范围为[0,num_key]，如果返回的key_idx=num_key，则表示target大于等于最后一个key
 * @note 注意此处的范围从1开始
 * YKY: 我的实现范围从0开始，假定key_no为num_key处的key大于任何有限的key
 */
int IxNodeHandle::upper_bound(const char *target, size_t pre) const {
    // Todo:
    // 查找当前节点中第一个大于target的key，并返回key的位置给上层
    // 提示:
    // 可以采用多种查找方式：顺序遍历、二分查找等；使用ix_compare()函数进行比较
    if (page_hdr->num_key == 0) return 0;
    const std::vector<ColType> &col_types = file_hdr->col_types_;
    const std::vector<int> &col_lens = file_hdr->col_lens_;
    pre = (pre == 0 ? static_cast<size_t>(file_hdr->col_num_) : pre);
    int l = 0, r = page_hdr->num_key - 1;
    while (l < r) {
        int mid = (l + r) >> 1;
        if (ix_compare(get_key(mid), target, col_types, col_lens, pre) <= 0) {
            l = mid + 1;
        } else {
            r = mid;
        }
    }
    if (ix_compare(get_key(r), target, col_types, col_lens, pre) > 0) {
        return r;
    }
    return page_hdr->num_key;
}

/**
 * @brief 用于叶子结点根据key来查找该结点中的键值对
 * 值value作为传出参数，函数返回是否查找成功
 *
 * @param key 目标key
 * @param[out] value 传出参数，目标key对应的Rid
 * @return 目标key是否存在
 */
bool IxNodeHandle::leaf_lookup(const char *key, Rid **value) {
    // Todo:
    // 1. 在叶子节点中获取目标key所在位置
    // 2. 判断目标key是否存在
    // 3. 如果存在，获取key对应的Rid，并赋值给传出参数value
    // 提示：可以调用lower_bound()和get_rid()函数。

    // 没有使用lower_bound，为了减少一次判断
    const std::vector<ColType> &col_types = file_hdr->col_types_;
    const std::vector<int> &col_lens = file_hdr->col_lens_;
    int l = 0, r = page_hdr->num_key - 1;
    size_t pre = static_cast<size_t>(file_hdr->col_num_);
    while (l < r) {
        int mid = (l + r) >> 1;
        if (ix_compare(get_key(mid), key, col_types, col_lens, pre) < 0) {
            l = mid + 1;
        } else {
            r = mid;
        }
    }
    if (ix_compare(get_key(r), key, col_types, col_lens, pre) == 0) {
        *value = get_rid(r);
        return true;
    }

    return false;
}

/**
 * 用于内部结点（非叶子节点）查找目标key所在的孩子结点（子树）
 * @param key 目标key
 * @return page_id_t 目标key所在的孩子节点（子树）的存储页面编号
 */
page_id_t IxNodeHandle::internal_lookup(const char *key, size_t pre) {
    // Todo:
    // 1. 查找当前非叶子节点中目标key所在孩子节点（子树）的位置
    // 2. 获取该孩子节点（子树）所在页面的编号
    // 3. 返回页面编号
    int slot_no = upper_bound(key, pre);
    return rids[slot_no].page_no;
}

/**
 * @brief 在指定位置插入n个连续的键值对
 * 将key的前n位插入到原来keys中的pos位置；将rid的前n位插入到原来rids中的pos位置
 *
 * @param pos 要插入键值对的位置
 * @param (key, rid) 连续键值对的起始地址，也就是第一个键值对，可以通过(key,
 * rid)来获取n个键值对
 * @param n 键值对数量
 * @note [0,pos)           [pos,num_key)
 *                            key_slot
 *                            /      \
 *                           /        \
 *       [0,pos)     [pos,pos+n)   [pos+n,num_key+n)
 *                      key           key_slot
 */

/* YKY：感觉不会用到
void IxNodeHandle::insert_pairs(int pos, const char *key, const Rid *rid,
                                int n) {
    // Todo:
    // 1. 判断pos的合法性
    // 2. 通过key获取n个连续键值对的key值，并把n个key值插入到pos位置
    // 3. 通过rid获取n个连续键值对的rid值，并把n个rid值插入到pos位置
    // 4. 更新当前节点的键数量
}
*/

/**
 * @brief
 * 用于在结点中插入单个键值对。目前仅仅针对叶子节点插入，函数返回插入后的键值对数量
 *
 * @param (key, value) 要插入的键值对
 * @return int 键值对数量
 */
int IxNodeHandle::insert(const char *key, const Rid &value) {
    // Todo:
    // 1. 查找要插入的键值对应该插入到当前节点的哪个位置
    // 2. 如果key重复则不插入
    // 3. 如果key不重复则插入键值对
    // 4. 返回完成插入操作之后的键值对数量
    assert(is_leaf_page());
    int key_idx = lower_bound(key);
    if (key_idx == page_hdr->num_key ||
        ix_compare(get_key(key_idx), key, file_hdr->col_types_, file_hdr->col_lens_) > 0) {
        insert_pair(key_idx, key, value);
    } else {
        assert(key_idx < page_hdr->num_key);
        throw IndexEntryNotUniqueError();
    }

    return page_hdr->num_key;
}

/**
 * @brief 用于在结点中的指定位置删除单个键值对
 *
 * @param pos 要删除键值对的位置
 */
void IxNodeHandle::erase_pair(int pos) {
    // Todo:
    // 1. 删除该位置的key
    // 2. 删除该位置的rid
    // 3. 更新结点的键值对数量
    assert(pos < page_hdr->num_key);
    assert(is_leaf_page());
    for (int i = pos; i < page_hdr->num_key - 1; ++i) {
        memcpy(get_key(i), get_key(i + 1), file_hdr->col_tot_len_);
        *get_rid(i) = *get_rid(i + 1);
    }
    --page_hdr->num_key;
}

/**
 * @brief 用于在结点中删除指定key的键值对。函数返回删除后的键值对数量
 *
 * @param key 要删除的键值对key值
 * @return 完成删除操作后的key数量
 */
int IxNodeHandle::remove(const char *key) {
    // Todo:
    // 1. 查找要删除键值对的位置
    // 2. 如果要删除的键值对存在，删除键值对
    // 3. 返回完成删除操作后的键值对数量
    assert(is_leaf_page());
    int key_idx = lower_bound(key);
    while (key_idx < get_size() && ix_compare(get_key(key_idx), key, file_hdr->col_types_, file_hdr->col_lens_) == 0) {
        // TODO:如果一次有多个删除的kv，可优化
        // page_hdr->num_key在该函数内部减1
        erase_pair(key_idx++);
    }

    return page_hdr->num_key;
}

IxIndexHandle::IxIndexHandle(DiskManager *disk_manager, BufferPoolManager *buffer_pool_manager, int fd)
    : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager), fd_(fd) {
    // YKY   bug?
    // init file_hdr_
    // disk_manager_->read_page(fd, IX_FILE_HDR_PAGE, (char *)&file_hdr_,
    // sizeof(file_hdr_));
    // bug end
    char *buf = new char[PAGE_SIZE];
    memset(buf, 0, PAGE_SIZE);
    disk_manager_->read_page(fd, IX_FILE_HDR_PAGE, buf, PAGE_SIZE);
    file_hdr_ = new IxFileHdr();
    file_hdr_->deserialize(buf);

    // YKY   bug?
    // disk_manager_->set_fd2pageno(fd, now_page_no + 1);
    // bug end
    disk_manager_->set_fd2pageno(fd, file_hdr_->num_pages_);
    delete[] buf;
}

/**
 * @brief 用于查找指定键所在的叶子结点
 * @param key 要查找的目标key值
 * @param operation 查找到目标键值对后要进行的操作类型
 * 不考虑叶子节点是否有该key，直接返回叶子节点即可
 * @param transaction 事务参数，如果不需要则默认传入nullptr
 * @return leaf node 返回目标叶子结点
 * 注意：find_leaf_page不对找到的叶子节点加锁
 */
IxNodeHandle *IxIndexHandle::find_leaf_page(const char *key, Operation operation, Transaction *transaction,
                                            bool find_first) {
    // Todo:
    // 1. 获取根节点
    // 2. 从根节点开始不断向下查找目标key
    // 3. 找到包含该key值的叶子结点停止查找，并返回叶子节点
    IxNodeHandle *tnode = fetch_node(file_hdr_->root_page_);
    if (tnode->is_leaf_page()) {
        return tnode;
    }

    while (true) {
        page_id_t page_id = tnode->internal_lookup(key);
        release_node_handle(tnode, false);

        tnode = fetch_node(page_id);
        if (tnode->is_leaf_page()) {
            break;
        }
    }
    return tnode;
}

bool IxIndexHandle::is_exist(const char *key, Transaction *transaction) {
    root_latch_.lock_shared();

    IxNodeHandle *ix_hdl = find_leaf_page(key, Operation::FIND, transaction);
    int key_idx = ix_hdl->lower_bound(key);
    if (key_idx < ix_hdl->get_size() &&
        ix_compare(key, ix_hdl->get_key(key_idx), file_hdr_->col_types_, file_hdr_->col_lens_) == 0) {
        release_node_handle(ix_hdl, false);
        root_latch_.unlock_shared();
        return true;
    }
    release_node_handle(ix_hdl, false);
    root_latch_.unlock_shared();
    return false;
}
/**
 * @brief 用于查找指定键在叶子结点中的对应的值result
 *
 * @param key 查找的目标key值
 * @param result 用于存放结果的容器
 * @param transaction 事务指针
 * @return bool 返回目标键值对是否存在
 */
bool IxIndexHandle::get_value(const char *key, std::vector<Rid> &result, Transaction *transaction) {
    // Todo:
    // 1. 获取目标key值所在的叶子结点
    // 2. 在叶子节点中查找目标key值的位置，并读取key对应的rid
    // 3. 把rid存入result参数中
    // 提示：使用完buffer_pool提供的page之后，记得unpin
    // page；记得处理并发的上锁
    result.clear();
    root_latch_.lock_shared();

    IxNodeHandle *ix_hdl = find_leaf_page(key, Operation::FIND, transaction);
    int key_idx = ix_hdl->lower_bound(key);
    while (key_idx < ix_hdl->get_size() &&
           ix_compare(key, ix_hdl->get_key(key_idx), file_hdr_->col_types_, file_hdr_->col_lens_) == 0) {
        result.push_back(*ix_hdl->get_rid(key_idx));
        ++key_idx;
    }
    release_node_handle(ix_hdl, false);
    root_latch_.unlock_shared();
    return !result.empty();
}

/**
 * @brief
 将传入的一个node拆分(Split)成两个结点，在node的右边生成一个新结点new
 * node，并将节点插入父节点（insert_into_parent）。在调用前对B+树加锁
 * @param node 需要拆分的结点
 * @return 拆分得到的new_node
 * @note：函数执行完毕后，原node和new node都需要在函数外面进行unpin
 *key   [0]                                                 [11]
 *     /   \_______________________________________________/    \
 *key ...  |    [1]        [3]     [5]     [7]     [9]     |    ...
 *         |(<)/   \(>=)  /   \   /   \   /   \   /   \    |
 *value:   |[0]       [2]      [4]     [6]     [8]     [10]|

 *key   [0]                         [5]                     [11]
 *     /   \________________________/ \____________________/    \
 *key ...  |    [1]        [3]      | |    [7]     [9]     |    ...
 *         |(<)/   \(>=)  /   \     | |   /   \   /   \    |
 *value:   |[0]       [2]      [4]  | |[6]     [8]     [10]|
 */
IxNodeHandle *IxIndexHandle::split(IxNodeHandle *node) {
    // Todo:
    // 1. 将原结点的键值对平均分配，右半部分分裂为新的右兄弟结点
    //    需要初始化新节点的page_hdr内容
    // 2.
    // 如果新的右兄弟结点是叶子结点，更新新旧节点的prev_leaf和next_leaf指针
    //    为新节点分配键值对，更新旧节点的键值对数记录
    // 3.
    // 如果新的右兄弟结点不是叶子结点，更新该结点的所有孩子结点的父节点信息(使用IxIndexHandle::maintain_child())
    assert(node->get_size() == node->get_max_key_size() + 1);
    IxNodeHandle *new_node = create_node();
    int split_from = node->get_min_key_size(), split_to = node->get_size();
    char *insert_up_key = node->get_key(split_from);
    if (node->is_leaf_page()) {
        memcpy(new_node->get_key(0), node->get_key(split_from), file_hdr_->col_tot_len_ * (split_to - split_from));
        memcpy(new_node->get_rid(0), node->get_rid(split_from), sizeof(Rid) * (split_to - split_from));

        *new_node->page_hdr = {
            .next_free_page_no = IX_NO_PAGE,
            .parent = node->get_parent_page_no(),
            .num_key = split_to - split_from,
            .is_leaf = true,
            .prev_leaf = node->get_page_no(),
            .next_leaf = node->get_next_leaf(),
        };

        node->set_size(split_from);
        if (node->get_next_leaf() != IX_NO_PAGE) {
            IxNodeHandle *old_next_node = fetch_node(node->get_next_leaf());
            old_next_node->set_prev_leaf(new_node->get_page_no());
            release_node_handle(old_next_node, true);
        } else {
            file_hdr_->last_leaf_ = new_node->get_page_no();
        }
        node->set_next_leaf(new_node->get_page_no());
    } else {
        memcpy(new_node->get_key(0), node->get_key(split_from + 1),
               file_hdr_->col_tot_len_ * (split_to - split_from - 1));
        memcpy(new_node->get_rid(0), node->get_rid(split_from + 1), file_hdr_->col_tot_len_ * (split_to - split_from));

        *new_node->page_hdr = {
            .next_free_page_no = IX_NO_PAGE,
            .parent = node->get_parent_page_no(),
            .num_key = split_to - split_from - 1,
            .is_leaf = false,
            .prev_leaf = node->get_page_no(),
            .next_leaf = node->get_next_leaf(),
        };

        node->set_size(split_from);
        if (node->get_next_leaf() != IX_NO_PAGE) {
            IxNodeHandle *old_next_node = fetch_node(node->get_next_leaf());
            old_next_node->set_prev_leaf(new_node->get_page_no());
            release_node_handle(old_next_node, true);
        }
        node->set_next_leaf(new_node->get_page_no());
        for (int i = 0; i < split_to - split_from; ++i) {
            maintain_child(new_node, i);
        }
    }

    insert_into_parent(node, insert_up_key, new_node, nullptr);

    return new_node;
}

/**
 * @brief Insert key & value pair into internal page after split
 * 拆分(Split)后，向上找到old_node的父结点
 * 将new_node的第一个key插入到父结点，其位置在 父结点指向old_node的孩子指针
 * 之后
 * 如果插入后>=maxsize，则必须继续拆分父结点，然后在其父结点的父结点再插入，即需要递归
 * 直到找到的old_node为根结点时，结束递归（此时将会新建一个根R，关键字为key，old_node和new_node为其孩子）
 *
 * @param (old_node, new_node)
 * 原结点为old_node，old_node被分裂之后产生了新的右兄弟结点new_node
 * @param key 要插入parent的key
 * @note
 * 一个结点插入了键值对之后需要分裂，分裂后左半部分的键值对保留在原结点，在参数中称为old_node，
 * 右半部分的键值对分裂为新的右兄弟节点，在参数中称为new_node（参考Split函数来理解old_node和new_node）
 * @note 本函数执行完毕后，new node和old node都需要在函数外面进行unpin
 * @note
 * 调用前加锁
 */
void IxIndexHandle::insert_into_parent(IxNodeHandle *old_node, const char *key, IxNodeHandle *new_node,
                                       Transaction *transaction) {
    // Todo:
    // 1. 分裂前的结点（原结点,
    // old_node）是否为根结点，如果为根结点需要分配新的root
    // 2. 获取原结点（old_node）的父亲结点
    // 3. 获取key对应的rid，并将(key, rid)插入到父亲结点
    // 4. 如果父亲结点仍需要继续分裂，则进行递归插入
    IxNodeHandle *parent_node = nullptr;
    if (old_node->is_root_page()) {
        parent_node = create_node();
        *(parent_node->page_hdr) = {
            .next_free_page_no = IX_NO_PAGE,
            .parent = IX_NO_PAGE,
            .num_key = 0,
            .is_leaf = false,
            .prev_leaf = IX_NO_PAGE,
            .next_leaf = IX_NO_PAGE,
        };

        memcpy(parent_node->get_key(0), key, parent_node->file_hdr->col_tot_len_);
        parent_node->get_rid(0)->page_no = old_node->get_page_no();
        parent_node->get_rid(1)->page_no = new_node->get_page_no();
        ++parent_node->page_hdr->num_key;

        old_node->set_parent_page_no(parent_node->get_page_no());
        new_node->set_parent_page_no(parent_node->get_page_no());
        file_hdr_->root_page_ = parent_node->get_page_no();
    } else {
        parent_node = fetch_node(old_node->get_parent_page_no());
        int key_idx = parent_node->upper_bound(key);
        for (int i = parent_node->get_size(); i > key_idx; --i) {
            memcpy(parent_node->get_key(i), parent_node->get_key(i - 1), file_hdr_->col_tot_len_);
            *parent_node->get_rid(i + 1) = *parent_node->get_rid(i);
        }

        memcpy(parent_node->get_key(key_idx), key, file_hdr_->col_tot_len_);
        *parent_node->get_rid(key_idx + 1) = {new_node->get_page_no(), -1};
        if ((++parent_node->page_hdr->num_key) > parent_node->get_max_key_size() + 1) {
            IxNodeHandle *parent_new_node = split(parent_node);
            release_node_handle(parent_new_node, true);
        }
    }
    release_node_handle(parent_node, true);
}

/**
 * @brief 将指定键值对插入到B+树中
 * @param (key, value) 要插入的键值对
 * @param transaction 事务指针
 * @return page_id_t 插入到的叶结点的page_no
 */
page_id_t IxIndexHandle::insert_entry(const char *key, const Rid &value, Transaction *transaction) {
    // Todo:
    // 1. 查找key值应该插入到哪个叶子节点
    // 2. 在该叶子节点中插入键值对
    // 3. 如果结点已满，分裂结点，并把新结点的相关信息插入父节点
    // 提示：记得unpin
    // page；若当前叶子节点是最右叶子节点，则需要更新file_hdr_.last_leaf；记得处理并发的上锁
    std::scoped_lock lock(root_latch_);
    IxNodeHandle *old_ix_hdl = find_leaf_page(key, Operation::INSERT, transaction);

    int pair_count = 0;
    try {
        pair_count = old_ix_hdl->insert(key, value);
    } catch (IndexEntryNotUniqueError &e) {
        release_node_handle(old_ix_hdl, false);
        throw e;
    }

    if (pair_count > old_ix_hdl->get_max_key_size()) {
        assert(pair_count == old_ix_hdl->get_max_key_size() + 1);
        IxNodeHandle *prev_node = nullptr;
        if (old_ix_hdl->get_prev_leaf() != IX_NO_PAGE) {
            prev_node = fetch_node(old_ix_hdl->get_prev_leaf());
            if (prev_node->get_size() < prev_node->get_max_key_size()) {
                redistribute(prev_node, old_ix_hdl);
                page_id_t insert_id =
                    (ix_compare(key, old_ix_hdl->get_key(0), file_hdr_->col_types_, file_hdr_->col_lens_) >= 0
                         ? old_ix_hdl->get_page_no()
                         : prev_node->get_page_no());
                release_node_handle(prev_node, true);
                release_node_handle(old_ix_hdl, true);
                return insert_id;
            }
            release_node_handle(prev_node, false);
        }

        IxNodeHandle *next_node = nullptr;
        if (old_ix_hdl->get_next_leaf() != IX_NO_PAGE) {
            next_node = fetch_node(old_ix_hdl->get_next_leaf());
            if (next_node->get_size() < next_node->get_max_key_size()) {
                redistribute(old_ix_hdl, next_node);
                page_id_t insert_id =
                    (ix_compare(key, next_node->get_key(0), file_hdr_->col_types_, file_hdr_->col_lens_) >= 0
                         ? next_node->get_page_no()
                         : old_ix_hdl->get_page_no());
                release_node_handle(next_node, true);
                release_node_handle(old_ix_hdl, true);
                return insert_id;
            }
            release_node_handle(next_node, false);
        }

        IxNodeHandle *new_ix_hdl = split(old_ix_hdl);
        page_id_t insert_id = ix_compare(key, new_ix_hdl->get_key(0), old_ix_hdl->file_hdr->col_types_,
                                         old_ix_hdl->file_hdr->col_lens_) >= 0
                                  ? new_ix_hdl->get_page_no()
                                  : old_ix_hdl->get_page_no();
        release_node_handle(new_ix_hdl, true);
        release_node_handle(old_ix_hdl, true);
        return insert_id;
    }
    page_id_t old_page_id = old_ix_hdl->get_page_no();
    release_node_handle(old_ix_hdl, true);
    return old_page_id;
}

/**
 * @brief 用于删除B+树中含有指定key的键值对
 * @note
 * 叶子节点old_ix_hdl加写锁并解锁，可能对该叶子节点以上的所有父辈结点加写锁
 * @param key 要删除的key值
 * @param transaction 事务指针
 */
bool IxIndexHandle::delete_entry(const char *key, Transaction *transaction) {
    // Todo:
    // 1. 获取该键值对所在的叶子结点
    // 2. 在该叶子结点中删除键值对
    // 3.
    // 如果删除成功需要调用CoalesceOrRedistribute来进行合并或重分配操作，并根据函数返回结果判断是否有结点需要删除
    // 4.
    // 如果需要并发，并且需要删除叶子结点，则需要在事务的delete_page_set中添加删除结点的对应页面；记得处理并发的上锁
    std::scoped_lock lock(root_latch_);
    IxNodeHandle *old_ix_hdl = find_leaf_page(key, Operation::DELETE, transaction);
    int pair_count = old_ix_hdl->remove(key);
    if (!old_ix_hdl->is_root_page() && pair_count < old_ix_hdl->get_min_key_size()) {
        // 如果没有删除old_ix_hdl，则需要unpin
        if (!coalesce_or_redistribute(old_ix_hdl)) {
            release_node_handle(old_ix_hdl, true);
        }
    } else {
        release_node_handle(old_ix_hdl, true);
    }
    return false;
}

/**
 * @brief 用于处理合并和重分配的逻辑，用于删除键值对后调用
 *
 * @param node 执行完删除操作的结点
 * @param transaction 事务指针
 * @param root_is_latched 传出参数：根节点是否上锁，用于并发操作
 * @return 是否需要删除结点node
 * @note User needs to first find the sibling of input page.
 * If sibling's size + input page's size >= 2 * page's minsize, then
 * redistribute. Otherwise, merge(Coalesce).
 */
bool IxIndexHandle::coalesce_or_redistribute(IxNodeHandle *node, Transaction *transaction, bool *root_is_latched) {
    // Todo:
    // 1. 判断node结点是否为根节点
    //    1.1 如果是根节点，需要调用AdjustRoot()
    //    函数来进行处理，返回根节点是否需要被删除 1.2
    //    如果不是根节点，并且不需要执行合并或重分配操作，则直接返回false，否则执行2
    // 2. 获取node结点的父亲结点
    // 3. 寻找node结点的兄弟结点（优先选取前驱结点）
    // 4.
    // 如果node结点和兄弟结点的键值对数量之和，能够支撑两个B+树结点（即node.size+neighbor.size
    // >= NodeMinSize*2)，则只需要重新分配键值对（调用Redistribute函数）
    // 5.
    // 如果不满足上述条件，则需要合并两个结点，将右边的结点合并到左边的结点（调用Coalesce函数）
    if (node->is_root_page()) {
        return adjust_root(node);
    } else {
        page_id_t left_node_id = node->get_prev_leaf();
        if (left_node_id != IX_NO_PAGE) {
            IxNodeHandle *left_node = fetch_node(left_node_id);
            if (left_node->get_size() > left_node->get_min_key_size()) {
                redistribute(left_node, node);
                release_node_handle(left_node, true);
                return false;
            } else {
                // 将node合并到左节点中，删除node
                coalesce(left_node, node);
                release_node_handle(left_node, true);
                return true;
            }
            release_node_handle(left_node, true);
        }

        page_id_t right_node_id = node->get_next_leaf();
        if (right_node_id != IX_NO_PAGE) {
            IxNodeHandle *right_node = fetch_node(right_node_id);
            if (right_node->get_size() > right_node->get_min_key_size()) {
                redistribute(node, right_node);
                release_node_handle(right_node, true);
                return false;
            } else {
                // 将右节点合并到node中，node不被删除
                coalesce(node, right_node);
                return false;
            }
        }
    }
    assert(false);
    return false;
}

/**
 * @brief 用于当根结点被删除了一个键值对之后的处理
 * @param old_root_node 原根节点
 * @return bool 根结点是否需要被删除
 * @note size of root page can be less than min size and this method is only
 * called within coalesce_or_redistribute()
 */
bool IxIndexHandle::adjust_root(IxNodeHandle *old_root_node) {
    // Todo:
    // 1.
    // 如果old_root_node不是叶子结点，并且有0个key，1个value，则直接把它的孩子更新成新的根结点
    // 2. 除了上述情况，不需要进行操作
    assert(!old_root_node->is_leaf_page());
    assert(old_root_node->get_size() == 0);
    page_id_t new_root_id = old_root_node->value_at(0);
    IxNodeHandle *new_root = fetch_node(new_root_id);
    buffer_pool_manager_->unpin_page(old_root_node->get_page_id(), false);
    buffer_pool_manager_->delete_page(old_root_node->get_page_id());
    file_hdr_->root_page_ = new_root_id;
    new_root->set_parent_page_no(IX_NO_PAGE);
    release_node_handle(new_root, true);
    return true;
}

// void IxIndexHandle::redistribute(IxNodeHandle *neighbor_node,
//                                  IxNodeHandle *node, IxNodeHandle
//                                  *parent, int index) {

/**
 * @brief 重新分配node和兄弟结点neighbor_node的键值对
 * Redistribute key & value pairs from one page to its sibling page. If
 * index == 0, move sibling page's first key & value pair into end of input
 * "node", otherwise move sibling page's last key & value pair into head of
 * input "node".
 *
 * @param neighbor_node sibling page of input "node"
 * @param node input from method coalesceOrRedistribute()
 * @param index node在parent中的rid_idx
 * @note node是之前刚被删除过一个key的结点
 * index=0，则neighbor是node后继结点，表示：node(left)      neighbor(right)
 * index>0，则neighbor是node前驱结点，表示：neighbor(left)  node(right)
 * 注意更新parent结点的相关kv对和孩字结点的父结点信息
 */
void IxIndexHandle::redistribute(IxNodeHandle *left_node, IxNodeHandle *right_node) {
    //  Todo:
    //  1. 通过index判断neighbor_node是否为node的前驱结点
    //  2. 从neighbor_node中移动键值对到node结点中
    //  3.
    //  更新父节点中的相关信息，并且修改移动键值对对应孩字结点的父结点信息（maintain_child函数）
    //  注意：neighbor_node的位置不同，需要移动的键值对不同，需要分类讨论
    int &left_size = left_node->page_hdr->num_key;
    int &right_size = right_node->page_hdr->num_key;
    assert((left_size - right_size > 1) || (right_size - left_size > 1));
    if (left_node->is_leaf_page()) {
        assert(right_node->is_leaf_page());
        if (left_size < right_size) {
            while (right_size - left_size > 1) {
                memcpy(left_node->get_key(left_size), right_node->get_key(0), file_hdr_->col_tot_len_);
                *left_node->get_rid(left_size) = *right_node->get_rid(0);
                ++left_size;
                // 右节点剩下的kv向前移
                for (int i = 0; i < right_size - 1; ++i) {
                    memcpy(right_node->get_key(i), right_node->get_key(i + 1), file_hdr_->col_tot_len_);
                    *right_node->get_rid(i) = *right_node->get_rid(i + 1);
                }
                --right_size;
            }
        } else {
            while (left_size - right_size > 1) {
                // 右节点所有kv右移
                for (int i = right_size; i > 0; --i) {
                    memcpy(right_node->get_key(i), right_node->get_key(i - 1), file_hdr_->col_tot_len_);
                    *right_node->get_rid(i) = *right_node->get_rid(i - 1);
                }
                memcpy(right_node->get_key(0), left_node->get_key(left_size - 1), file_hdr_->col_tot_len_);
                *right_node->get_rid(0) = *left_node->get_rid(left_size - 1);
                ++right_size;
                --left_size;
            }
        }
        maintain_parent(right_node);
        return;
    }
    // 非叶子节点
    if (left_size < right_size) {
        while (right_size - left_size > 1) {
            // 将右节点的第一个kv移动到左节点的最后，修改key为右节点的最小key值
            IxNodeHandle *tmp = get_min_leafnode(right_node);
            memcpy(left_node->get_key(left_size), tmp->get_key(0), file_hdr_->col_tot_len_);
            release_node_handle(tmp, false);
            *left_node->get_rid(left_size + 1) = *right_node->get_rid(0);
            // 右节点剩下的kv向前移
            for (int i = 0; i < right_size - 1; ++i) {
                memcpy(right_node->get_key(i), right_node->get_key(i + 1), file_hdr_->col_tot_len_);
                *right_node->get_rid(i) = *right_node->get_rid(i + 1);
            }
            *right_node->get_rid(right_size - 1) = *right_node->get_rid(right_size);
            ++left_size;
            --right_size;
            maintain_child(left_node, left_size);
        }
    } else {
        while (left_size - right_size > 1) {
            // 获取右节点的最小key值，必须放在这里，因为后面对右节点进行了修改
            IxNodeHandle *tmp = get_min_leafnode(right_node);
            // 右节点的kv向后移
            for (int i = right_size; i > 0; --i) {
                memcpy(right_node->get_key(i), right_node->get_key(i - 1), file_hdr_->col_tot_len_);
                *right_node->get_rid(i + 1) = *right_node->get_rid(i);
            }
            *right_node->get_rid(1) = *right_node->get_rid(0);
            // 将左节点的最后一个kv移动到右节点的头部，修改key为右节点最小key值
            memcpy(right_node->get_key(0), tmp->get_key(0), file_hdr_->col_tot_len_);
            release_node_handle(tmp, false);
            *right_node->get_rid(0) = *left_node->get_rid(left_size);
            ++right_size;
            --left_size;
            maintain_child(right_node, 0);
        }
    }
    maintain_parent(right_node);
}

/**
 * @brief
 * 合并(Coalesce)函数是将node和其直接前驱进行合并，也就是和它左边的left_node进行合并；
 * Move all the key & value pairs from one page to its sibling page, and
 * notify buffer pool manager to delete this page. Parent page must be
 * adjusted to take info of deletion into account. Remember to deal with
 * coalesce or redistribute recursively if necessary.
 *
 * @param left_node
 * @param right_node input from method coalesceOrRedistribute()
 * (right_node结点是需要被删除的)
 * @param parent parent page of input "node"
 * @param index node在parent中的rid_idx
 * @return true means parent node should be deleted, false means no deletion
 * happend
 * @note Assume that *neighbor_node is the left sibling of *node (neighbor
 * -> node)
 */
void IxIndexHandle::coalesce(IxNodeHandle *left_node, IxNodeHandle *right_node, Transaction *transaction,
                             bool *root_is_latched) {
    // Todo:
    // 1.
    // 用index判断neighbor_node是否为node的前驱结点，若不是则交换两个结点，让neighbor_node作为左结点，node作为右结点
    // 2.
    // 把node结点的键值对移动到neighbor_node中，并更新node结点孩子结点的父节点信息（调用maintain_child函数）
    // 3.
    // 释放和删除node结点，并删除parent中node结点的信息，返回parent是否需要被删除
    // 提示：如果是叶子结点且为最右叶子结点，需要更新file_hdr_.last_leaf
    int &left_size = left_node->page_hdr->num_key;
    int &right_size = right_node->page_hdr->num_key;
    if (left_node->is_leaf_page()) {
        // leaf节点，则key size之和应小于或等于最大允许key size
        assert(left_size + right_size <= left_node->get_max_key_size());
        assert(right_node->is_leaf_page());
        assert(left_node->get_next_leaf() == right_node->get_page_no());
        // 修改叶子节点的链表结构
        page_id_t rright_node_id = right_node->get_next_leaf();
        left_node->set_next_leaf(rright_node_id);
        if (rright_node_id != IX_NO_PAGE) {
            IxNodeHandle *rright_node = fetch_node(rright_node_id);
            rright_node->set_prev_leaf(left_node->get_page_no());
            release_node_handle(rright_node, true);
        } else {
            file_hdr_->last_leaf_ = left_node->get_page_no();
        }
        // 将右节点的所有kv移动到左节点，更新左节点的size
        for (int i = left_size, j = 0; j < right_size; ++i, ++j) {
            memcpy(left_node->get_key(i), right_node->get_key(j), file_hdr_->col_tot_len_);
            *left_node->get_rid(i) = *right_node->get_rid(j);
        }
        left_node->page_hdr->num_key += right_size;
        // 删除右节点页，更新右节点的父节点的kv，删去其中和右节点相关的kv，可能需要进一步更新祖先节点
        update_right_parent(right_node);
        return;
    }

    // internal节点，则key size之和应小于最大允许key size，因为还需要新建一个key
    assert(left_size + right_size < left_node->get_max_key_size());
    // 更新链表结构
    page_id_t rright_node_id = right_node->get_next_leaf();
    left_node->set_next_leaf(rright_node_id);
    if (rright_node_id != IX_NO_PAGE) {
        IxNodeHandle *rright_node = fetch_node(rright_node_id);
        rright_node->set_prev_leaf(left_node->get_page_no());
        release_node_handle(rright_node, true);
    }
    // 新建的key值应为右子节点的最小key值
    IxNodeHandle *right_min_node = get_min_leafnode(right_node);
    memcpy(left_node->get_key(left_size), right_min_node->get_key(0), file_hdr_->col_tot_len_);
    // 将右节点的kv复制到左节点
    for (int i = left_size + 1, j = 0; j < right_size; ++i, ++j) {
        memcpy(left_node->get_key(i), right_node->get_key(j), file_hdr_->col_tot_len_);
        *left_node->get_rid(i) = *right_node->get_rid(j);
    }
    *left_node->get_rid(left_size + right_size + 1) = *right_node->get_rid(right_size);
    // 更新新加入左节点的孩子的父节点信息
    for (int i = left_size + 1; i <= left_size + right_size + 1; ++i) {
        maintain_child(left_node, i);
    }
    left_size += right_size + 1;
    // 删除右节点页，更新右节点的父节点的kv，删去其中和右节点相关的kv，可能需要进一步更新祖先节点
    update_right_parent(right_node);
}

/**
 * @brief:coalesce函数的辅助函数，删除右节点页，更新右节点的父节点的kv，删去其中和右节点相关的kv，可能需要进一步更新祖先节点
 */
void IxIndexHandle::update_right_parent(IxNodeHandle *right_node) {
    page_id_t right_parent_no = right_node->get_parent_page_no();
    IxNodeHandle *right_parent = fetch_node(right_parent_no);
    int child_idx = right_parent->find_child(right_node);
    buffer_pool_manager_->unpin_page(right_node->get_page_id(), false);
    buffer_pool_manager_->delete_page(right_node->get_page_id());
    int &right_parent_size = right_parent->page_hdr->num_key;
    if (child_idx == 0) {
        for (int i = 0; i < right_parent_size - 1; ++i) {
            memcpy(right_parent->get_key(i), right_parent->get_key(i + 1), file_hdr_->col_tot_len_);
            *right_parent->get_rid(i) = *right_parent->get_rid(i + 1);
        }
        *right_parent->get_rid(right_parent_size - 1) = *right_parent->get_rid(right_parent_size);
        --right_parent_size;
        maintain_parent(right_parent);
    } else {
        for (int i = child_idx - 1; i < right_parent_size - 1; ++i) {
            memcpy(right_parent->get_key(i), right_parent->get_key(i + 1), file_hdr_->col_tot_len_);
            *right_parent->get_rid(i + 1) = *right_parent->get_rid(i + 2);
        }
        --right_parent_size;
    }

    if ((right_parent->is_root_page() && right_parent_size == 0) ||
        (!right_parent->is_root_page() && right_parent_size < right_parent->get_min_key_size())) {
        // 如果没有删除right_parent节点，则还需要unpin
        if (!coalesce_or_redistribute(right_parent)) {
            release_node_handle(right_parent, true);
        }
    } else {
        release_node_handle(right_parent, true);
    }
}

/**
 * @brief 这里把iid转换成了rid，即iid的slot_no作为node的rid_idx(key_idx)
 * node其实就是把slot_no作为键值对数组的下标
 * 换而言之，每个iid对应的索引槽存了一对(key,rid)，指向了(要建立索引的属性首地址,插入/删除记录的位置)
 *
 * @param iid
 * @return Rid
 * @note
 * iid和rid存的不是一个东西，rid是上层传过来的记录位置，iid是索引内部生成的索引槽位置
 */
Rid IxIndexHandle::get_rid(const Iid &iid) const {
    IxNodeHandle *node = fetch_node(iid.page_no);
    if (iid.slot_no == node->get_size()) {
        // 当upper/lower
        // bound返回key_num时（大于所有key）会出现这种情况，不能报异常
        release_node_handle(node, false);  // unpin it!
        return {-1, -1};
    } else if (iid.slot_no > node->get_size()) {
        throw IndexEntryNotFoundError();
    }

    Rid res = *node->get_rid(iid.slot_no);

    release_node_handle(node, false);  // unpin it!
    return res;
}

/**
 * @brief FindLeafPage + lower_bound
 *
 * @param key
 * @return Iid
 * @note 上层传入的key本来是int类型，通过(const char *)&key进行了转换
 * 可用*(int *)key转换回去
 */
Iid IxIndexHandle::lower_bound(const char *key, size_t pre) {
    IxNodeHandle *leaf_hdl = find_leaf_page(key, Operation::FIND, nullptr);
    int slot_no = leaf_hdl->lower_bound(key, pre);
    page_id_t page_id = -1;
    if (slot_no == leaf_hdl->page_hdr->num_key && leaf_hdl->get_next_leaf() != IX_NO_PAGE) {
        slot_no = 0;
        page_id = leaf_hdl->get_next_leaf();
    } else {
        page_id = leaf_hdl->get_page_no();
    }
    release_node_handle(leaf_hdl, false);
    return {page_id, slot_no};
}

/**
 * @brief FindLeafPage + upper_bound
 *
 * @param key
 * @return Iid
 */
Iid IxIndexHandle::upper_bound(const char *key, size_t pre) {
    IxNodeHandle *leaf_hdl = find_leaf_page(key, Operation::FIND, nullptr);
    int slot_no = leaf_hdl->upper_bound(key, pre);
    page_id_t page_id = -1;
    if (slot_no == leaf_hdl->page_hdr->num_key && leaf_hdl->get_next_leaf() != IX_NO_PAGE) {
        slot_no = 0;
        page_id = leaf_hdl->get_next_leaf();
    } else {
        page_id = leaf_hdl->get_page_no();
    }
    release_node_handle(leaf_hdl, false);
    return {page_id, slot_no};
}

/**
 * @brief 指向最后一个叶子的最后一个结点的后一个
 * 用处在于可以作为IxScan的最后一个
 *
 * @return Iid
 */
Iid IxIndexHandle::leaf_end() const {
    IxNodeHandle *node = fetch_node(file_hdr_->last_leaf_);
    Iid iid = {.page_no = file_hdr_->last_leaf_, .slot_no = node->get_size()};
    release_node_handle(node, false);  // unpin it!
    return iid;
}

/**
 * @brief 指向第一个叶子的第一个结点
 * 用处在于可以作为IxScan的第一个
 *
 * @return Iid
 */
Iid IxIndexHandle::leaf_begin() const {
    Iid iid = {.page_no = file_hdr_->first_leaf_, .slot_no = 0};
    return iid;
}

/**
 * @brief 获取一个指定结点
 *
 * @param page_no
 * @return IxNodeHandle*
 * @note pin the page, remember to unpin it outside!
 */
IxNodeHandle *IxIndexHandle::fetch_node(int page_no) const {
    Page *page = buffer_pool_manager_->fetch_page(PageId{fd_, page_no});
    IxNodeHandle *node = new IxNodeHandle(file_hdr_, page);

    return node;
}

/**
 * @brief 创建一个新结点
 *
 * @return IxNodeHandle*
 * @note pin the page, remember to unpin it outside!
 * 注意：对于Index的处理是，删除某个页面后，认为该被删除的页面是free_page
 * 而first_free_page实际上就是最新被删除的页面，初始为IX_NO_PAGE
 * 在最开始插入时，一直是create
 * node，那么first_page_no一直没变，一直是IX_NO_PAGE
 * 与Record的处理不同，Record将未插入满的记录页认为是free_page
 */
IxNodeHandle *IxIndexHandle::create_node() {
    IxNodeHandle *node;
    file_hdr_->num_pages_++;

    PageId new_page_id = {.fd = fd_, .page_no = INVALID_PAGE_ID};
    if (file_hdr_->first_free_page_no_ != IX_NO_PAGE) {
        new_page_id.page_no = file_hdr_->first_free_page_no_;
        Page *page = buffer_pool_manager_->fetch_page(new_page_id);
        // 先不处理page_hdr，由调用函数处理
        node = new IxNodeHandle(file_hdr_, page);
        file_hdr_->first_free_page_no_ = node->page_hdr->next_free_page_no;
        return node;
    }
    Page *page = buffer_pool_manager_->new_page(&new_page_id);
    node = new IxNodeHandle(file_hdr_, page);
    return node;
}

/**
 * @brief
 * 从node开始更新其父节点的第一个key，一直向上更新直到根节点，如果传入的node和找到的最小key节点不一样，则函数内部unpin找到的min_leafnode，否则不unpin
 *
 * @param node
 */
void IxIndexHandle::maintain_parent(IxNodeHandle *node) {
    IxNodeHandle *curr = node;
    page_id_t parent_page_no = curr->get_parent_page_no();
    assert(parent_page_no != IX_NO_PAGE);
    IxNodeHandle *min_leafnode = get_min_leafnode(node);
    const char *min_key = min_leafnode->get_key(0);
    while (parent_page_no != IX_NO_PAGE) {
        // Load its parent
        IxNodeHandle *parent = fetch_node(parent_page_no);
        int rank = parent->find_child(curr);
        if (curr != node) {
            release_node_handle(curr, false);
        }

        if (rank > 0) {
            char *parent_key = parent->get_key(rank - 1);
            memcpy(parent_key, min_key, file_hdr_->col_tot_len_);
            release_node_handle(parent, true);
            break;
        }
        parent_page_no = parent->get_parent_page_no();
        curr = parent;
    }
    if (min_leafnode != node) {
        release_node_handle(min_leafnode, false);
    }
}

/**
 * @brief
 * 要删除leaf之前调用此函数，更新leaf前驱结点的next指针和后继结点的prev指针
 *
 * @param leaf 要删除的leaf
 */
void IxIndexHandle::erase_leaf(IxNodeHandle *leaf) {
    assert(leaf->is_leaf_page());

    IxNodeHandle *prev = fetch_node(leaf->get_prev_leaf());
    prev->set_next_leaf(leaf->get_next_leaf());
    release_node_handle(prev, true);

    IxNodeHandle *next = fetch_node(leaf->get_next_leaf());
    next->set_prev_leaf(leaf->get_prev_leaf());  // 注意此处是SetPrevLeaf()
    release_node_handle(next, true);
}

/**
 * @brief 删除node时，更新file_hdr_.num_pages
 *
 * @param node
 */
void IxIndexHandle::release_node_handle(IxNodeHandle *node, bool is_dirty) const {
    if (buffer_pool_manager_->unpin_page(node->get_page_id(), is_dirty))
        delete node;
    else
        assert(false);
}

/**
 * @brief 将node的第child_idx个孩子结点的父节点置为node
 */
void IxIndexHandle::maintain_child(IxNodeHandle *node, int child_idx) {
    if (!node->is_leaf_page()) {
        //  Current node is inner node, load its child and set its parent to
        //  current node
        int child_page_no = node->value_at(child_idx);
        IxNodeHandle *child = fetch_node(child_page_no);
        child->set_parent_page_no(node->get_page_no());
        release_node_handle(child, true);
    }
}