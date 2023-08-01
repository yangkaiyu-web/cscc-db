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
#include <fcntl.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <memory>
#include <ratio>
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

struct TupleBufFile {
    std::string file_name_;
    int fd_;
    size_t tuple_len_;
    size_t tot_read_tuple_num;
    size_t tot_write_tuple_num;
    TupleBufFile() = default;
    TupleBufFile(std::string& file_name, size_t tuple_len) : file_name_(file_name), tuple_len_(tuple_len) {
        tot_read_tuple_num = 0;
        tot_write_tuple_num = 0;
        ::close(::open(file_name_.c_str(), O_CREAT | O_RDWR, 0777));
    }

    // 以可读写方式打开，默认文件已存在
    int open() {
        fd_ = ::open(file_name_.c_str(), O_RDWR, 0777);
        assert(fd_ != -1);
        return fd_;
    }

    size_t write(char* buffer, size_t max_write_tuple_num) {
        size_t writed_size = ::write(fd_, buffer, tuple_len_ * max_write_tuple_num);
        assert(writed_size > 0 && writed_size % tuple_len_ == 0);
        size_t write_tuple_num = writed_size / tuple_len_;
        tot_write_tuple_num += write_tuple_num;
        return write_tuple_num;
    }

    size_t read(char* buf, size_t max_read_tuple_num) {
        size_t read_size = ::read(fd_, buf, tuple_len_ * max_read_tuple_num);
        assert(read_size > 0 && read_size % tuple_len_ == 0);
        size_t read_tuple_num = read_size / tuple_len_;
        tot_read_tuple_num += read_tuple_num;
        return read_tuple_num;
    }

    int close() {
        tot_read_tuple_num = 0;
        fd_ = -1;
        return ::close(fd_);
    }

    bool is_eof() { return tot_write_tuple_num == tot_read_tuple_num; }

    bool is_opened() { return fd_ != -1; }

    ~TupleBufFile() { ::unlink(file_name_.c_str()); }
};

class SortExecutor : public AbstractExecutor {
   private:
    // b    k      m    g
    static const size_t tuple_num_per_step = 2000;
    std::unique_ptr<AbstractExecutor> prev_;
    // pair.second为true时是desc，仅为参与排序的列，全部列可从prev_->cols()获得
    std::vector<std::pair<ColMeta, bool>> cols_;  // 框架中只支持一个键排序，需要自行修改数据结构支持多个键排序
    size_t total_tuple_num_;
    int tuple_len_;
    int limit_num_;

    // 存储最终排序好的结果
    std::unique_ptr<TupleBufFile> res_file_;
    char* buffer0_;
    // buffer0的实际size，并不总是等于tuple_num_per_step，比如最后一部分数据不一定装满
    size_t tuple_num_in_buf0_;
    // 调用Next方法时加一，是buffer0索引指针
    size_t curr_read_idx_;

    bool is_end_;

   public:
    SortExecutor(std::unique_ptr<AbstractExecutor> prev, std::vector<std::pair<TabCol, bool>> sel_cols,
                 int limit_num = -1) {
        prev_ = std::move(prev);
        tuple_len_ = prev_->tupleLen();
        for (auto& col : sel_cols) {
            cols_.push_back(std::make_pair(prev_->get_col_offset(col.first), col.second));
        }

        total_tuple_num_ = 0;
        limit_num_ = limit_num;
        is_end_ = false;
        buffer0_ = new char[tuple_len_ * tuple_num_per_step];
        prev_->beginTuple();
        // TODO:后续可删除
        if (!prev_->is_end()) {
            assert(prev_->Next()->size == tuple_len_);
        }
        // part data
        size_t tuple_num = 0;
        std::vector<std::unique_ptr<RmRecord>> tuples;
        tuples.reserve(tuple_num_per_step);
        std::vector<std::unique_ptr<TupleBufFile>> tmp_files;

        auto func = [this](std::unique_ptr<RmRecord>& t1, std::unique_ptr<RmRecord>& t2) {
            return this->cmp_record(t1, t2);
        };
        while (!prev_->is_end()) {
            tuples.emplace_back(prev_->Next());
            tuple_num++;
            if (tuple_num == tuple_num_per_step) {
                std::sort(tuples.begin(), tuples.end(), func);
                for (size_t i = 0; i < tuples.size(); ++i) {
                    memcpy(buffer0_ + i * tuple_len_, tuples[i]->data, tuple_len_);
                }
                writeTmpData(buffer0_, tmp_files, tuple_num_per_step);
                tuples.clear();
                total_tuple_num_ += tuple_num_per_step;
                tuple_num = 0;
            }
            prev_->nextTuple();
        }
        if (tuple_num != 0) {
            total_tuple_num_ += tuple_num;
            std::sort(tuples.begin(), tuples.end(), func);
            for (size_t i = 0; i < tuples.size(); ++i) {
                memcpy(buffer0_ + i * tuple_len_, tuples[i]->data, tuple_len_);
            }
            writeTmpData(buffer0_, tmp_files, tuple_num);
            tuples.clear();
        }

        char* buffer1 = new char[tuple_len_ * tuple_num_per_step];
        char* buffer2 = new char[tuple_len_ * tuple_num_per_step];
        res_file_ = mergeTmpFiles(tmp_files, 0, tmp_files.size() - 1, buffer0_, buffer1, buffer2);
        delete[] buffer1;
        delete[] buffer2;
    }

    void beginTuple() override {
        // 每次调用beginTuple都置零
        curr_read_idx_ = 0;
        if (res_file_ == nullptr || limit_num_ == 0) {
            is_end_ = true;
            return;
        }
        if (res_file_->is_opened()) {
            res_file_->close();
        }
        res_file_->open();

        tuple_num_in_buf0_ = res_file_->read(buffer0_, tuple_num_per_step);

        // 如果一个tuple也没有，直接标记end
        if (tuple_num_in_buf0_ == 0) {
            if (res_file_->is_eof()) {
                is_end_ = true;
            } else {
                // 没有结束但是读不出来tuple
                assert(false);
            }
        }
    }

    /*
    // 如果是 desc， 那么大的先落盘，
    int find_element(std::vector<std::unique_ptr<RmRecord>>& datas) {
        auto tmp = datas.begin();
        int index = 0;
        for (size_t i = 1; i < datas.size(); i++) {
            auto rec = datas.begin() + i;
            int ret = cmp(*tmp, *rec, cols_);
            if (ret > 0) {
                tmp = rec;
                index = i;
            }
        }
        return index;
    }*/

    void nextTuple() override {
        ++curr_read_idx_;
        if (limit_num_ > 0 && curr_read_idx_ == static_cast<size_t>(limit_num_)) {
            is_end_ = true;
            return;
        }
        if (curr_read_idx_ == tuple_num_in_buf0_) {
            if (res_file_->is_eof()) {
                is_end_ = true;
                return;
            }
            tuple_num_in_buf0_ = res_file_->read(buffer0_, tuple_num_per_step);
            curr_read_idx_ = 0;
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        return std::make_unique<RmRecord>(tuple_len_, buffer0_ + curr_read_idx_ * tuple_len_);
    }

    bool is_end() const override { return is_end_; }

    int tupleLen() const override { return tuple_len_; }

    const std::vector<ColMeta>& cols() const override { return prev_->cols(); }

    std::string getType() override { return "SortExecutor"; }

    ColMeta get_col_offset(const TabCol& target) override { return prev_->get_col_offset(target); };

    Rid& rid() override { return _abstract_rid; }

    ~SortExecutor() { delete[] buffer0_; }

   private:
    bool cmp_record(const std::unique_ptr<RmRecord>& rec1, const std::unique_ptr<RmRecord>& rec2) {
        for (auto& col : cols_) {
            auto val1 = Value::read_from_record(rec1, col.first);
            auto val2 = Value::read_from_record(rec2, col.first);
            if (val1 > val2) {
                return (col.second == true);

            } else if (val1 < val2) {
                return (col.second == false);
            }
        }
        return true;
    }

    bool cmp_raw_data(const char* fir_data, const char* sec_data) {
        for (auto& col : cols_) {
            switch (col.first.type) {
                case TYPE_INT: {
                    const int val1 = *(const int*)(fir_data + col.first.offset);
                    const int val2 = *(const int*)(sec_data + col.first.offset);
                    if (val1 > val2) {
                        return col.second == false;
                    } else if (val1 < val2) {
                        return col.second == true;
                    }
                    break;
                }
                case TYPE_BIGINT: {
                    const int64_t val1 = *(const int64_t*)(fir_data + col.first.offset);
                    const int64_t val2 = *(const int64_t*)(sec_data + col.first.offset);
                    if (val1 > val2) {
                        return col.second == false;
                    } else if (val1 < val2) {
                        return col.second == true;
                    }
                    break;
                }
                case TYPE_DATETIME: {
                    const uint64_t val1 = *(const uint64_t*)(fir_data + col.first.offset);
                    const uint64_t val2 = *(const uint64_t*)(sec_data + col.first.offset);
                    if (val1 > val2) {
                        return col.second == false;
                    } else if (val1 < val2) {
                        return col.second == true;
                    }
                    break;
                }
                case TYPE_FLOAT: {
                    const float val1 = *(const float*)(fir_data + col.first.offset);
                    const float val2 = *(const float*)(sec_data + col.first.offset);
                    if (val1 > val2) {
                        return col.second == false;
                    } else if (val1 < val2) {
                        return col.second == true;
                    }
                    break;
                }
                case TYPE_STRING: {
                    const char* val1 = fir_data + col.first.offset;
                    const char* val2 = sec_data + col.first.offset;
                    int res = strcmp(val1, val2);
                    if (res > 0) {
                        return col.second == false;
                    } else if (res < 0) {
                        return col.second == true;
                    }
                    break;
                }
                default: {
                    throw RMDBError("type not found");
                    break;
                }
            }
        }
        return true;
    }

    std::string getTime() {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::duration<long long, std::nano> timestamp =
            std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
        return std::to_string(timestamp.count());
    }

    void writeTmpData(char* buffer, std::vector<std::unique_ptr<TupleBufFile>>& tmp_files, size_t tuple_num) {
        std::string tmp_file_name = "sort.tmp" + getTime();

        auto tmp_file = std::make_unique<TupleBufFile>(tmp_file_name, tuple_len_);
        if (tmp_file->open() < 0) {
            throw UnixError();
        }

        tmp_file->write(buffer, tuple_num);
        tmp_file->close();
        tmp_files.emplace_back(std::move(tmp_file));
    }

    // 归并，单线程
    std::unique_ptr<TupleBufFile> mergeTmpFiles(std::vector<std::unique_ptr<TupleBufFile>>& tmp_files, int start_idx,
                                                int end_idx, char* buffer0, char* buffer1, char* buffer2) {
        if (start_idx > end_idx) {
            return nullptr;
        }
        if (start_idx == end_idx) {
            return std::move(tmp_files[start_idx]);
        }
        std::unique_ptr<TupleBufFile> fir_file = nullptr, sec_file = nullptr;
        int mid = (start_idx + end_idx) >> 1;
        fir_file = mergeTmpFiles(tmp_files, start_idx, mid, buffer0, buffer1, buffer2);
        sec_file = mergeTmpFiles(tmp_files, mid + 1, end_idx, buffer0, buffer1, buffer2);

        std::string tmp_file_name = "sort.tmp" + getTime();
        auto ret_file = std::make_unique<TupleBufFile>(tmp_file_name, tuple_len_);

        size_t buf0_idx = 0, buf1_idx = 0, buf2_idx = 0;

        fir_file->open();
        sec_file->open();
        ret_file->open();
        size_t tuple_num_in_buf0 = fir_file->read(buffer0, tuple_num_per_step);
        size_t tuple_num_in_buf1 = sec_file->read(buffer1, tuple_num_per_step);

        while (true) {
            if (buf0_idx == tuple_num_in_buf0) {
                if (fir_file->is_eof()) {
                    break;
                }
                buf0_idx = 0;
                tuple_num_in_buf0 = fir_file->read(buffer0, tuple_num_per_step);
            }

            if (buf1_idx == tuple_num_in_buf1) {
                if (sec_file->is_eof()) {
                    break;
                }
                buf1_idx = 0;
                tuple_num_in_buf1 = sec_file->read(buffer1, tuple_num_per_step);
            }

            char* fir_ptr = buffer0 + buf0_idx * tuple_len_;
            char* sec_ptr = buffer1 + buf1_idx * tuple_len_;
            // 返回1的在前
            if (cmp_raw_data(fir_ptr, sec_ptr)) {
                memcpy(buffer2 + buf2_idx * tuple_len_, fir_ptr, tuple_len_);
                ++buf0_idx;
            } else {
                memcpy(buffer2 + buf2_idx * tuple_len_, sec_ptr, tuple_len_);
                ++buf1_idx;
            }
            ++buf2_idx;
            if (buf2_idx == tuple_num_per_step) {
                ret_file->write(buffer2, tuple_num_per_step);
                buf2_idx = 0;
            }
        }
        if (buf2_idx) ret_file->write(buffer2, buf2_idx);

        while (true) {
            if (buf0_idx == tuple_num_in_buf0) {
                if (fir_file->is_eof()) {
                    break;
                }
                buf0_idx = 0;
                tuple_num_in_buf0 = fir_file->read(buffer0, tuple_num_per_step);
            }
            char* fir_ptr = buffer0 + buf0_idx * tuple_len_;
            ret_file->write(fir_ptr, tuple_num_in_buf0 - buf0_idx);
            buf0_idx = tuple_num_in_buf0;
        }

        while (true) {
            if (buf1_idx == tuple_num_in_buf1) {
                if (sec_file->is_eof()) {
                    break;
                }
                buf1_idx = 0;
                tuple_num_in_buf1 = fir_file->read(buffer1, tuple_num_per_step);
            }
            char* sec_ptr = buffer1 + buf1_idx * tuple_len_;
            ret_file->write(sec_ptr, tuple_num_in_buf1 - buf1_idx);
            buf1_idx = tuple_num_in_buf1;
        }
        fir_file->close();
        sec_file->close();
        ret_file->close();
        return ret_file;
    }
};
