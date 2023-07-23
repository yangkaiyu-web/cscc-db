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
    SmManager* sm_manager_;
    std::unique_ptr<AbstractExecutor> prev_;
    std::vector<ColMeta> cols_;  // 框架中只支持一个键排序，需要自行修改数据结构支持多个键排序
    size_t tuple_num;
    int limit_num_;
    bool is_desc_;
    std::vector<size_t> used_tuple;
    std::unique_ptr<RmRecord> current_tuple;

   public:
    SortExecutor(SmManager* sm_manager, std::unique_ptr<AbstractExecutor> prev, std::vector<TabCol> sel_cols,
                 bool is_desc, int limit_num) {
        prev_ = std::move(prev);
        for (auto& col : sel_cols) {
            cols_.push_back(prev_->get_col_offset(col));
        }
        is_desc_ = is_desc;
        tuple_num = 0;
        limit_num_ = limit_num;
        used_tuple.clear();
    }

    int cmp(std::shared_ptr<RmRecord>& rec1, std::shared_ptr<RmRecord>& rec2, std::vector<ColMeta>& cols) {
        int ret = 0;
        for (auto col : cols) {
            auto val1 = Value::read_from_record(rec1, col);
            auto val2 = Value::read_from_record(rec2, col);
            if (val1 > val2) {
                ret = 1;
                break;
            } else if (val1 < val2) {
                ret = -1;
                break;
            }
        }
        return ret;
    }
    void sort(char* buf, int tuple_num) {
        auto tuple_len = prev_->tupleLen();
        // TODO:低效，反复申请新内存，如何利用起来已经获取的 page
        for (int i = 0; i < tuple_num - 1; i++) {
            for (int j = 0; j < tuple_num - i - 1; j++) {
                auto rec1 = std::make_unique<RmRecord>(prev_->tupleLen(), buf + (j)*tuple_len);
                auto rec2 = std::make_unique<RmRecord>(prev_->tupleLen(), buf + (j + 1) * tuple_len);
                auto ret = cmp(rec1, rec2, cols_);
                if (is_desc_) ret = -ret;
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
    void writeTmpData(char* read_buf,int tmp_file_count, std::vector<std::string>& tmp_file_names) {
                std::string tmp_filename = "sort" + std::to_string(tmp_file_count) + ".tmp" + getTime();
        std::ofstream tmpfile(tmp_filename,std::ios::binary);
        tmpfile.write(read_buf, PAGE_SIZE);
        tmpfile.close();
                tmp_file_names.push_back(tmp_filename);
    }
    // TODO: 缓存一下排序结果，防止反复 begin
    void beginTuple() override {
        std::vector<std::string> tmp_file_names;
        int tmp_file_count = 0;
        auto tuple_num_per_page = PAGE_SIZE / prev_->tupleLen();
        auto tuple_len = prev_->tupleLen();
        prev_->beginTuple();
        int tuple_num_on_page = 0;
        char read_buf[PAGE_SIZE];
        // part data
        while (!prev_->is_end()) {
            auto tuple = prev_->Next();
            memcpy(read_buf + tuple_len * tuple_num_on_page, tuple->data, tuple_len);
            tuple_num_on_page++;
            if (tuple_num_on_page == tuple_num_per_page) {
                sort(read_buf, tuple_num_on_page);
                writeTmpData(read_buf, tmp_file_count++, tmp_file_names);
                tuple_num_on_page = 0;
            }
            prev_->nextTuple();
        }
        if (tuple_num_on_page != 0) {
            sort(read_buf, tuple_num_on_page);
            writeTmpData(read_buf, tmp_file_count++, tmp_file_names);
            tuple_num_on_page = 0;
        }


        // merge
        std::string res_file_name = "sort.res"+getTime();
        std::vector<std::string> res_file_names;
        std::vector<std::ifstream> streams;
        std::vector<std::shared_ptr<RmRecord>> datas;
        while(res_file_names.size()>1){
            auto block_it = tmp_file_names.begin();
            while(block_it!=tmp_file_names.end()){
                std::ifstream inputFile(*block_it,std::ios::binary);
                streams.push_back(std::move(inputFile));
                auto rec = std::make_shared<RmRecord>(prev_->tupleLen());
                streams.back().read(rec->data, prev_->tupleLen());
                if(streams.back().good()){
                    datas.push_back(rec);
                }
                if(streams.size()==tuple_num_on_page){
                    std::string output_file_name = "sort.tmp" + getTime();
                    std::ofstream output_file (output_file_name,std::ios::binary);
                    res_file_names.push_back(output_file_name);

                    while (!datas.empty()) {
                        auto minLineIt = find_element(datas);
                        int minLineIndex = minLineIt - currentLines.begin();
                        outputFile << currentLines[minLineIndex] << "\n";

                        // 读取对应块文件的下一行
                        if (getline(streams[minLineIndex], currentLines[minLineIndex])) {
                            continue;
                        }

                        // 当前块文件已经读取完，关闭文件并从内存中移除
                        streams[minLineIndex].close();
                        currentLines.erase(currentLines.begin() + minLineIndex);
                        streams.erase(streams.begin() + minLineIndex);
                    }

                    outputFile.close();

                }

            }


        }
    }
    // 如果是 desc， 那么大的先落盘，
    int find_element(std::vector<std::shared_ptr<RmRecord>> datas){
        auto tmp = datas.front();
        int index = 0;
        for(auto i =1;i<datas.size();i++){
            auto rec = datas[i];
            int ret = cmp(tmp,rec,cols_);
            if(is_desc_) ret = -ret;
            if(ret < 0){
                tmp  = rec;
                index = i;
            }
        }
    }
    // TODO: 缓存一下排序结果，防止反复 begin
    void beginTuple() override {
        std::vector<std::string> temp_file_names;
        int tmp_file_count = 0;
        auto tuple_num_per_page = PAGE_SIZE / prev_->tupleLen();
        auto tuple_len = prev_->tupleLen();
        prev_->beginTuple();
        int tuple_num_on_page = 0;
        char read_buf[PAGE_SIZE];
        while (!prev_->is_end()) {
            auto tuple = prev_->Next();
            memcpy(read_buf + tuple_len * tuple_num_on_page, tuple->data, tuple_len);
            tuple_num_on_page++;
            if (tuple_num_on_page == tuple_num_per_page) {
                sort(read_buf, tuple_num_on_page);
                writeTmpData(read_buf, tmp_file_count++, temp_file_names);
                tuple_num_on_page = 0;
            }
            prev_->nextTuple();
        }
        if (tuple_num_on_page != 0) {
            sort(read_buf, tuple_num_on_page);
            writeTmpData(read_buf, tmp_file_count++, temp_file_names);
            tuple_num_on_page = 0;
        }
        std::string res_file_name = "sort.res"+getTime();


void mergeBlocks(const vector<string>& blockFiles, const string& outputFilename) {
    
    // 打开每个块文件并读取第一行到内存
    for (const auto& blockFile : blockFiles) {
        ifstream inputFile(blockFile);
        if (inputFile.is_open()) {
            streams.push_back(move(inputFile));
            string line;
            if (getline(streams.back(), line)) {
                currentLines.push_back(line);
            }
        }
    }
    
    // 创建输出文件
    ofstream outputFile(outputFilename);
    
    // 通过比较当前行选择最小值并写入输出文件，直到所有块的数据被处理完
    while (!currentLines.empty()) {
        auto minLineIt = min_element(currentLines.begin(), currentLines.end());
        int minLineIndex = minLineIt - currentLines.begin();
        outputFile << currentLines[minLineIndex] << "\n";
        
        // 读取对应块文件的下一行
        if (getline(streams[minLineIndex], currentLines[minLineIndex])) {
            continue;
        }
        
        // 当前块文件已经读取完，关闭文件并从内存中移除
        streams[minLineIndex].close();
        currentLines.erase(currentLines.begin() + minLineIndex);
        streams.erase(streams.begin() + minLineIndex);
    }
    
    outputFile.close();
}

    void nextTuple() override {}

    std::unique_ptr<RmRecord> Next() override { return nullptr; }

    size_t tupleLen() const override { throw UnreachableCodeError(); }

    const std::vector<ColMeta>& cols() const override { throw UnreachableCodeError(); }
    std::string getType() override { return "SortExecutor"; }

    bool is_end() const override {
        // TODO: implement this
        return true;
    }

    Rid& rid() override { return _abstract_rid; }
};
