/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "ix_scan.h"

/**
 * @brief
 * @todo 加上读锁（需要使用缓冲池得到page）
 */
void IxScan::next() {
    assert(!is_end());
    // increment slot no
    iid_.slot_no++;
    if (iid_.page_no != ih_->file_hdr_->last_leaf_ && iid_.slot_no == curr_node_size_) {
        IxNodeHandle *node = ih_->fetch_node(iid_.page_no);
        // go to next leaf
        iid_.slot_no = 0;
        iid_.page_no = node->get_next_leaf();
        ih_->release_node_handle(node, false);

        node = ih_->fetch_node(iid_.page_no);
        assert(node->is_leaf_page());
        curr_node_size_ = node->get_size();
        ih_->release_node_handle(node, false);
    } else if (iid_.page_no == ih_->file_hdr_->last_leaf_ && iid_.slot_no == curr_node_size_) {
        is_end_ = true;
    }
}

Rid IxScan::rid() const { return ih_->get_rid(iid_); }