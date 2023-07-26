/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once
#include <cstring>
#include <ctime>
#include <fstream>
#include <memory>
#include <string>
#include <utility>

#include "common/config.h"
#include "errors.h"
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "record/rm_defs.h"
#include "storage/disk_manager.h"
#include "system/sm.h"
#include "system/sm_manager.h"

class SortExecutor : public AbstractExecutor {
   private:
    // TODO: 直接申请内存还是从bufferpool里获取？

    static const int mem_num = 10;  // 花费 10 页内存用于排序  5 页用于缓存输入， 5页用于缓存输出

    std::unique_ptr<AbstractExecutor> prev_;
    std::vector<std::pair<ColMeta, bool>> cols_;  // 框架中只支持一个键排序，需要自行修改数据结构支持多个键排序
    int tuple_num;
    int used_tuple_num;
    int limit_num_;

    std::ifstream res_file_;
    std::string res_file_name_;
    std::vector<size_t> used_tuple;
    RmRecord current_tuple_;

   public:
    SortExecutor(std::unique_ptr<AbstractExecutor> prev, std::vector<std::pair<TabCol, bool>> sel_cols, int limit_num) {
        prev_ = std::move(prev);
        for (auto& col : sel_cols) {
            cols_.push_back(std::make_pair(prev_->get_col_offset(col.first), col.second));
        }

        tuple_num = 0;
        used_tuple_num = 0;
        limit_num_ = limit_num;
        used_tuple.clear();
    }

    int cmp(std::unique_ptr<RmRecord>& rec1, std::unique_ptr<RmRecord>& rec2,
            std::vector<std::pair<ColMeta, bool>>& cols) {
        int ret = 0;
        for (auto col : cols) {
            auto val1 = Value::read_from_record(rec1, col.first);
            auto val2 = Value::read_from_record(rec2, col.first);
            if (val1 > val2) {
                ret = 1;

            } else if (val1 < val2) {
                ret = -1;
            }
            if (ret != 0) {
                if (col.second) {
                    ret = -ret;
                }
                break;
            }
        }
        return ret;
    }
    void sort(char* buf, int tuple_number) {
        auto tuple_len = prev_->tupleLen();
        // TODO:低效，反复申请新内存，如何利用起来已经获取的 page
        for (int i = 0; i < tuple_number - 1; i++) {
            for (int j = 0; j < tuple_number - i - 1; j++) {
                auto rec1 = std::make_unique<RmRecord>(prev_->tupleLen(), buf + (j)*tuple_len);
                auto rec2 = std::make_unique<RmRecord>(prev_->tupleLen(), buf + (j + 1) * tuple_len);
                auto ret = cmp(rec1, rec2, cols_);
                if (ret > 0) {  // more bigger more  backer
                    //  tmp = a[j]
                    memcpy(rec1->data, buf + (j)*tuple_len, prev_->tupleLen());
                    //  a[j ] = a[j+1]
                    memcpy(buf + (j)*tuple_len, buf + (j + 1) * tuple_len, prev_->tupleLen());
                    //  a[j+1 ] = tmp
                    memcpy(buf + (j + 1) * tuple_len, rec1->data, prev_->tupleLen());
                }
            }
        }
    }
    std::string getTime() {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::duration<long long, std::milli> timestamp =
            std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch();
        return std::to_string(timestamp.count());
    }
    void writeTmpData(char* read_buf, int tmp_file_count, std::vector<std::string>& tmp_file_names) {
        std::string tmp_filename = "sort" + std::to_string(tmp_file_count) + ".tmp" + getTime();
        std::ofstream tmpfile(tmp_filename, std::ios::binary);
        tmpfile.write(read_buf, PAGE_SIZE);
        tmpfile.close();
        tmp_file_names.push_back(tmp_filename);
    }
    // TODO: 缓存一下排序结果，防止反复 begin
    void beginTuple() override {
        tuple_num = 0;
        used_tuple_num = 0;
        std::vector<std::string> tmp_file_names;
        int tmp_file_count = 0;
        auto tuple_num_per_page = PAGE_SIZE / prev_->tupleLen();
        size_t tuple_len = prev_->tupleLen();
        prev_->beginTuple();
        size_t tuple_num_on_page = 0;
        char read_buf[PAGE_SIZE];
        // part data
        while (!prev_->is_end()) {
            auto tuple = prev_->Next();
            memcpy(read_buf + tuple_len * tuple_num_on_page, tuple->data, tuple_len);
            tuple_num_on_page++;
            if (tuple_num_on_page == tuple_num_per_page) {
                sort(read_buf, tuple_num_on_page);
                writeTmpData(read_buf, tmp_file_count++, tmp_file_names);
                tuple_num += tuple_num_on_page;
                tuple_num_on_page = 0;
            }
            prev_->nextTuple();
        }
        if (tuple_num_on_page != 0) {
            tuple_num += tuple_num_on_page;
            sort(read_buf, tuple_num_on_page);
            writeTmpData(read_buf, tmp_file_count++, tmp_file_names);
            tuple_num_on_page = 0;
        }

        // merge
        std::vector<std::string> res_file_names;
        std::vector<std::ifstream> streams;
        std::vector<std::unique_ptr<RmRecord>> datas;
        // std::copy(tmp_file_names.begin(), tmp_file_names.end(), res_file_names.begin());
        res_file_names = tmp_file_names;
        while (res_file_names.size() > 1) {
            tmp_file_names.clear();
            // std::copy(res_file_names.begin(), res_file_names.end(), tmp_file_names.begin());
            tmp_file_names = res_file_names;
            res_file_names.clear();
            auto block_it = tmp_file_names.begin();
            while (block_it != tmp_file_names.end()) {
                std::ifstream inputFile(*block_it, std::ios::binary);
                streams.push_back(std::move(inputFile));
                auto rec = std::make_unique<RmRecord>(prev_->tupleLen());
                streams.back().read(rec->data, prev_->tupleLen());
                if (streams.back().good()) {
                    datas.push_back(std::move(rec));
                }

                if (streams.size() == tuple_num_on_page) {
                    std::string output_file_name = "sort.tmp" + getTime();
                    std::ofstream output_file(output_file_name, std::ios::binary);
                    res_file_names.push_back(output_file_name);

                    while (!datas.empty()) {
                        auto index = find_element(datas);

                        output_file.write(datas[index]->data, prev_->tupleLen());

                        // 读取对应块文件的下一行

                        streams[index].read(datas[index]->data, prev_->tupleLen());
                        if (streams.back().good()) {
                            continue;
                        }

                        // 当前块文件已经读取完，关闭文件并从内存中移除
                        streams[index].close();
                        if (remove(block_it->c_str()) != 0) {
                            throw UnixError();
                        }
                        datas.erase(datas.begin() + index);
                        streams.erase(streams.begin() + index);
                    }
                    assert(datas.empty());
                    assert(streams.empty());
                    output_file.close();
                }
                block_it++;
            }
            if (!streams.empty()) {
                std::string output_file_name = "sort.tmp" + getTime();
                std::ofstream output_file(output_file_name, std::ios::binary);
                res_file_names.push_back(output_file_name);

                while (!datas.empty()) {
                    auto index = find_element(datas);

                    output_file.write(datas[index]->data, prev_->tupleLen());

                    // 读取对应块文件的下一行

                    streams[index].read(datas[index]->data, prev_->tupleLen());
                    if (streams.back().good()) {
                        continue;
                    }

                    // 当前块文件已经读取完，关闭文件并从内存中移除
                    streams[index].close();
                    datas.erase(datas.begin() + index);
                    streams.erase(streams.begin() + index);
                }
                assert(datas.empty());
                assert(streams.empty());
                output_file.close();
            }
            tmp_file_names.clear();
            // std::copy(tmp_file_names.begin(), tmp_file_names.end(), res_file_names.begin());
            res_file_names = tmp_file_names;
        }
        assert(res_file_names.size() == 1);
        res_file_name_ = res_file_names[0];
        res_file_ = std::ifstream(res_file_name_, std::ios::binary);
        current_tuple_ = RmRecord(prev_->tupleLen());

        res_file_.read(current_tuple_.data, prev_->tupleLen());
        used_tuple_num++;
    }

    // 如果是 desc， 那么大的先落盘，
    int find_element(std::vector<std::unique_ptr<RmRecord>>& datas) {
        auto tmp = datas.begin();
        int index = 0;
        for (size_t i = 1; i < datas.size(); i++) {
            auto rec = datas.begin() + i;
            int ret = cmp(*tmp, *rec, cols_);
            if (ret < 0) {
                tmp = rec;
                index = i;
            }
        }
        return index;
    }

    void nextTuple() override {
        if (used_tuple_num < tuple_num || (limit_num_ > 0 && used_tuple_num < limit_num_)) {
            res_file_.read(current_tuple_.data, prev_->tupleLen());
            used_tuple_num++;
        } else if (used_tuple_num == tuple_num || (limit_num_ > 0 && used_tuple_num == limit_num_)) {
            res_file_.close();
            if (remove(res_file_name_.c_str()) != 0) {
                throw UnixError();
            }
            used_tuple_num++;
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        std::unique_ptr<RmRecord> ptr = std::make_unique<RmRecord>(current_tuple_);
        return ptr;
    }

    size_t tupleLen() const override { return prev_->tupleLen(); }

    const std::vector<ColMeta>& cols() const override { return prev_->cols(); }
    std::string getType() override { return "SortExecutor"; }

    ColMeta get_col_offset(const TabCol& target) override { return prev_->get_col_offset(target); };
    bool is_end() const override {
        bool not_finish;
        if (limit_num_ < 0) {
            not_finish = used_tuple_num <= tuple_num;
        } else if (limit_num_ >= 0) {
            not_finish = used_tuple_num <= limit_num_;
        } else {
            throw InternalError("limit number error");
        }
        return !not_finish;
    }

    Rid& rid() override { return _abstract_rid; }
};
