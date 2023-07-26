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
    std::string file_name;
    int fd;
    size_t tuple_len;
    size_t total_tuple_num;
    size_t written_num;
    size_t read_num;
    TupleBufFile(std::string& file_name_, size_t tuple_len_) : file_name(file_name_), tuple_len(tuple_len_) {
        total_tuple_num = written_num = read_num = 0;
    }
    TupleBufFile() : file_name(""), tuple_len(0) {
        total_tuple_num = written_num = read_num = 0;
    }
    int open_write() {
        fd = open(file_name.c_str(), O_CREAT | O_RDWR,0777);
        return fd;
    }
    int open_read() {
        fd = open(file_name.c_str(), O_RDWR,0777);
        return fd;
    }
    size_t write(const char* buf, size_t tuple_num) {
        size_t num = ::write(fd, buf, tuple_num * tuple_len);
        written_num += tuple_num;
        return num/tuple_len;
    }
    size_t read(char* buf, size_t tuple_num) {
        size_t num = ::read(fd, buf, tuple_num * tuple_len);
        read_num += tuple_num;
        return num/tuple_len;
    }
    int close() { return ::close(fd); }
    bool is_eof() { return written_num == read_num; }
};
class SortExecutor : public AbstractExecutor {
   private:
    // TODO: 直接申请内存还是从bufferpool里获取？

    static const int mem_num = 10;  // 花费 10 页内存用于排序  5 页用于缓存输入， 5页用于缓存输出

    std::unique_ptr<AbstractExecutor> prev_;
    std::vector<std::pair<ColMeta, bool>> cols_;  // 框架中只支持一个键排序，需要自行修改数据结构支持多个键排序
    size_t tuple_num_;
    int used_tuple_num;
    int limit_num_;

    std::shared_ptr<TupleBufFile> res_file_;
    std::string res_file_name_;
    std::vector<size_t> used_tuple;
    RmRecord current_tuple_;

   public:
    SortExecutor(std::unique_ptr<AbstractExecutor> prev, std::vector<std::pair<TabCol, bool>> sel_cols, int limit_num) {
        prev_ = std::move(prev);
        for (auto& col : sel_cols) {
            cols_.push_back(std::make_pair(prev_->get_col_offset(col.first), col.second));
        }

        tuple_num_ = 0;
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
    void sort(char* buf, size_t tuple_number) {
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
        std::chrono::duration<long long, std::nano> timestamp =
            std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch();
        return std::to_string(timestamp.count());
    }
    void writeTmpData(char* read_buf, std::vector<std::shared_ptr<TupleBufFile>>& tmp_files, size_t tuple_count) {
        std::string tmp_file_name = "sort.tmp" + getTime();

        auto  tmp_file = std::make_shared<TupleBufFile>(tmp_file_name, prev_->tupleLen());
        if (tmp_file->open_write() < 0) {
            throw UnixError();
        }
        if (tmp_file->write(read_buf, tuple_count) != tuple_count ) {
            throw UnixError();
        }
        tmp_file->close();
        tmp_files.push_back(tmp_file);
    }
    // TODO: 缓存一下排序结果，防止反复 begin
    void beginTuple() override {
        tuple_num_ = 0;
        used_tuple_num = 0;
        std::vector<std::string> tmp_file_names;
        std::vector<std::shared_ptr<TupleBufFile>> tmp_files;

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
                writeTmpData(read_buf, tmp_files, tuple_num_on_page);
                tuple_num_ += tuple_num_on_page;
                tuple_num_on_page = 0;
            }
            prev_->nextTuple();
        }
        if (tuple_num_on_page != 0) {
            tuple_num_ += tuple_num_on_page;
            sort(read_buf, tuple_num_on_page);
            writeTmpData(read_buf, tmp_files, tuple_num_on_page);
            tuple_num_on_page = 0;
        }

        // merge


        std::vector<std::unique_ptr<RmRecord>> datas;
        // std::copy(tmp_file_names.begin(), tmp_file_names.end(), res_file_names.begin());

        while (tmp_files.size() > 1) {
            std::vector<std::shared_ptr<TupleBufFile>> used_files;
            // std::copy(res_file_names.begin(), res_file_names.end(), tmp_file_names.begin());
            auto block_it = tmp_files.begin();
            while (block_it != tmp_files.end()) {
                used_files.push_back(*block_it);
                auto rec = std::make_unique<RmRecord>(prev_->tupleLen());
                if(used_files.back()->open_read()<0){
                    throw UnixError();
                }
                if (used_files.back()->read(rec->data, 1) != 1) {
                    throw UnixError();
                }
                if (used_files.back()->read_num <= used_files.back()->written_num) {
                    datas.push_back(std::move(rec));
                } else {
                    throw InternalError("error");
                }

                if (used_files.size() == tuple_num_per_page) {
                    mergeTmpFiles(datas, used_files, tmp_files);
                }
                block_it++;
            }
            if (!used_files.empty()) {
                mergeTmpFiles(datas, used_files, tmp_files);
            }


            // std::copy(tmp_file_names.begin(), tmp_file_names.end(), res_file_names.begin());
        }
        assert(tmp_files.size() == 1);
        res_file_ = tmp_files[0];
        res_file_->open_read();
        current_tuple_ = RmRecord(prev_->tupleLen());

        res_file_->read(current_tuple_.data, 1);
        used_tuple_num++;
    }

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
    }
    void mergeTmpFiles(std::vector<std::unique_ptr<RmRecord>>& datas, std::vector<std::shared_ptr<TupleBufFile>>& streams,
                       std::vector<std::shared_ptr<TupleBufFile>>& res_files) {
        std::string output_file_name = "sort.tmp" + getTime();
        auto  output_file = std::make_shared<TupleBufFile>(output_file_name, prev_->tupleLen());

        output_file->open_write();
        while (!datas.empty()) {
            auto index = find_element(datas);

            output_file->write(datas[index]->data, 1);

            // 读取对应块文件的下一行

            streams[index]->read(datas[index]->data, 1);
            if (streams[index]->read_num <=streams[index]->written_num) {
                continue;
            }

            // 当前块文件已经读取完，关闭文件并从内存中移除
            streams[index]->close();
            if(remove(streams[index]->file_name.c_str())<0){
                throw UnixError();
            }
            for(auto bb=res_files.begin();bb!=res_files.end();bb++){
                if((*bb)->file_name == (*(streams.begin()+index))->file_name){
                    res_files.erase(bb);
                    break;
                }
            }
            datas.erase(datas.begin() + index);
            streams.erase(streams.begin() + index);
        }
        assert(datas.empty());
        assert(streams.empty());
        output_file->close();
        res_files.push_back(output_file);
    }

    void nextTuple() override {
        if (used_tuple_num < tuple_num_ || (limit_num_ > 0 && used_tuple_num < limit_num_)) {
            res_file_->read(current_tuple_.data, 1);
            used_tuple_num++;
        } else if (used_tuple_num == tuple_num_ || (limit_num_ > 0 && used_tuple_num == limit_num_)) {
            if (used_tuple_num == tuple_num_) {
                assert(res_file_->is_eof());
            }
            res_file_->close();
            if (remove(res_file_->file_name.c_str()) != 0) {
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
            not_finish = used_tuple_num <= tuple_num_;
        } else if (limit_num_ >= 0) {
            not_finish = used_tuple_num <= limit_num_;
        } else {
            throw InternalError("limit number error");
        }
        return !not_finish;
    }

    Rid& rid() override { return _abstract_rid; }
};
