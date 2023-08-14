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
#include "ix_index_handle.h"

// class IxIndexHandle;

// 用于遍历叶子结点
// 用于直接遍历叶子结点，而不用findleafpage来得到叶子结点
// TODO：对page遍历时，要加上读锁
class IxScan : public RecScan {
    IxIndexHandle *ih_;
    Iid iid_;  // 初始为lower（用于遍历的指针）
    Iid end_;  // 初始为upper
    BufferPoolManager *bpm_;
    bool is_end_;
    int curr_node_size_;

   public:
    IxScan(IxIndexHandle *ih, const Iid &lower, const Iid &upper, BufferPoolManager *bpm, bool is_end = false)
        : ih_(ih), iid_(lower), end_(upper), bpm_(bpm), is_end_(is_end), curr_node_size_(-1) {
        if (!(is_end_ || iid_ == end_)) {
            IxNodeHandle *node = ih_->fetch_node(iid_.page_no);
            assert(node->is_leaf_page());
            curr_node_size_ = node->get_size();
            ih_->release_node_handle(node, false);
        }
    }

    void next() override;

    // 目前索引的end判断有三种情况:
    // 1.正常到达end_
    // 2.查找条件大于record的最大值或小于record的最小值，lower bound==upper bound，也满足iid_==end_
    // 3.upper bound小于lower bound
    bool is_end() const override { return is_end_ || iid_ == end_; }

    Rid rid() const override;

    const Iid &iid() const { return iid_; }
};