/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sm_manager.h"

#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

#include "errors.h"
#include "index/ix.h"
#include "record/rm.h"
#include "record_printer.h"
#include "system/sm_meta.h"

/**
 * @description: 判断是否为一个文件夹
 * @return {bool} 返回是否为一个文件夹
 * @param {string&} db_name 数据库文件名称，与文件夹同名
 */
bool SmManager::is_dir(const std::string& db_name) {
    struct stat st;
    return stat(db_name.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

/**
 * @description: 创建数据库，所有的数据库相关文件都放在数据库同名文件夹下
 * @param {string&} db_name 数据库名称
 */
void SmManager::create_db(const std::string& db_name) {
    if (is_dir(db_name)) {
        throw DatabaseExistsError(db_name);
    }
    // 为数据库创建一个子目录
    std::string cmd = "mkdir " + db_name;
    if (system(cmd.c_str()) < 0) {  // 创建一个名为db_name的目录
        throw UnixError();
    }
    if (chdir(db_name.c_str()) < 0) {  // 进入名为db_name的目录
        throw UnixError();
    }
    // 创建系统目录
    DbMeta* new_db = new DbMeta();
    new_db->name_ = db_name;

    // 注意，此处ofstream会在当前目录创建(如果没有此文件先创建)和打开一个名为DB_META_NAME的文件
    std::ofstream ofs(DB_META_NAME);

    // 将new_db中的信息，按照定义好的operator<<操作符，写入到ofs打开的DB_META_NAME文件中
    ofs << *new_db;  // 注意：此处重载了操作符<<

    delete new_db;

    // 创建日志文件
    disk_manager_->create_file(LOG_FILE_NAME);

    // 回到根目录
    if (chdir("..") < 0) {
        throw UnixError();
    }
}

/**
 * @description: 删除数据库，同时需要清空相关文件以及数据库同名文件夹
 * @param {string&} db_name 数据库名称，与文件夹同名
 */
void SmManager::drop_db(const std::string& db_name) {
    if (!is_dir(db_name)) {
        throw DatabaseNotFoundError(db_name);
    }
    std::string cmd = "rm -r " + db_name;
    if (system(cmd.c_str()) < 0) {
        throw UnixError();
    }
}

/**
 * @description:
 * 打开数据库，找到数据库对应的文件夹，并加载数据库元数据和相关文件
 * @param {string&} db_name 数据库名称，与文件夹同名
 */
void SmManager::open_db(const std::string& db_name) {
    if (!is_dir(db_name)) {
        throw DatabaseNotFoundError(db_name);
    }
    if (chdir(db_name.c_str()) < 0) {  // 进入名为db_name的目录
        throw UnixError();
    }
    std::ifstream ofs(DB_META_NAME);
    // 无需加锁
    ofs >> db_;
    for (auto& tab : db_.tabs_) {
        // 无需加锁
        fhs_.emplace(tab.first, rm_manager_->open_file(tab.first));
        for (auto& index : tab.second.indexes) {
            const std::string& index_name = index.get_index_name();
            // 无需加锁
            ihs_.emplace(index_name, ix_manager_->open_index(index_name));
        }
    }
}

/**
 * @description: 把数据库相关的元数据刷入磁盘中
 */
void SmManager::flush_meta() {
    // 默认清空文件
    std::ofstream ofs(DB_META_NAME);
    ofs << db_;
}

/**
 * @description: 关闭数据库并把数据落盘
 */
void SmManager::close_db() {
    flush_meta();
    // 无需加锁
    for (auto& table : fhs_) {
        rm_manager_->close_file(table.second.get());
    }
    // 无需加锁
    for (auto& index : ihs_) {
        ix_manager_->close_index(index.second.get());
    }
}

/**
 * @description:
 * 显示所有的表,通过测试需要将其结果写入到output.txt,详情看题目文档
 * @param {Context*} context
 */
void SmManager::show_tables(Context* context) {
    std::fstream outfile;
    outfile.open("output.txt", std::ios::out | std::ios::app);
    outfile << "| Tables |\n";
    RecordPrinter printer(1);
    printer.print_separator(context);
    printer.print_record({"Tables"}, context);
    printer.print_separator(context);
    db_.RLatch();
    for (const auto& entry : db_.tabs_) {
        const auto& tab = entry.second;
        printer.print_record({tab.name}, context);
        outfile << "| " << tab.name << " |\n";
    }
    db_.RUnLatch();
    printer.print_separator(context);
    outfile.close();
}

void SmManager::show_indexes(const std::string& tab_name, Context* context) {
    std::fstream outfile;
    outfile.open("output.txt", std::ios::out | std::ios::app);
    RecordPrinter printer(1);
    printer.print_separator(context);
    printer.print_record({"Indexes"}, context);
    printer.print_separator(context);
    db_.RLatch();
    const std::vector<IndexMeta>& indexes = db_.get_table(tab_name).indexes;
    for (const IndexMeta& idx : indexes) {
        outfile << "| " << tab_name << " | unique | ";
        const std::vector<ColMeta>& cols_meta = idx.cols;
        std::string idx_cols = "(";
        for (size_t i = 0; i < cols_meta.size(); ++i) {
            idx_cols += cols_meta[i].name;
            idx_cols += (i == cols_meta.size() - 1 ? ")" : ",");
        }
        outfile << idx_cols + " |\n";
        printer.print_record({idx_cols}, context);
    }
    db_.RUnLatch();
    printer.print_separator(context);
    outfile.close();
}

/**
 * @description: 显示表的元数据
 * @param {string&} tab_name 表名称
 * @param {Context*} context
 */
void SmManager::desc_table(const std::string& tab_name, Context* context) {
    db_.RLatch();
    const TabMeta& tab = db_.get_table(tab_name);

    std::vector<std::string> captions = {"Field", "Type", "Index"};
    RecordPrinter printer(captions.size());
    // Print header
    printer.print_separator(context);
    printer.print_record(captions, context);
    printer.print_separator(context);
    // Print fields
    for (auto& col : tab.cols) {
        std::vector<std::string> field_info = {col.name, coltype2str(col.type), col.index ? "YES" : "NO"};
        printer.print_record(field_info, context);
    }
    db_.RUnLatch();
    // Print footer
    printer.print_separator(context);
}

/**
 * @description: 创建表
 * @param {string&} tab_name 表的名称
 * @param {vector<ColDef>&} col_defs 表的字段
 * @param {Context*} context
 */
void SmManager::create_table(const std::string& tab_name, const std::vector<ColDef>& col_defs, Context* context) {
    if (db_.is_table(tab_name)) {
        throw TableExistsError(tab_name);
    }
    // Create table meta
    int curr_offset = 0;
    TabMeta tab;
    tab.name = tab_name;
    for (auto& col_def : col_defs) {
        ColMeta col = {.tab_name = tab_name,
                       .name = col_def.name,
                       .type = col_def.type,
                       .len = col_def.len,
                       .offset = curr_offset,
                       .index = false};
        curr_offset += col_def.len;
        tab.cols.push_back(col);
    }
    // Create & open record file
    int record_size = curr_offset;  // record_size就是col
                                    // meta所占的大小（表的元数据也是以记录的形式进行存储的）
    rm_manager_->create_file(tab_name, record_size);
    db_.WLatch();
    db_.tabs_[tab_name] = tab;
    flush_meta();
    db_.WUnLatch();
    // fhs_[tab_name] = rm_manager_->open_file(tab_name);
    std::unique_ptr<RmFileHandle> fh = rm_manager_->open_file(tab_name);
    latch_.lock();
    fhs_.emplace(tab_name, std::move(fh));
    latch_.unlock();
}

/**
 * @description: 删除表
 * @param {string&} tab_name 表的名称
 * @param {Context*} context
 */
void SmManager::drop_table(const std::string& tab_name, Context* context) {
    for(auto& index_meta : db_.get_table(tab_name).indexes){
        drop_index(tab_name,index_meta.cols,context);
    }
    if (!db_.is_table(tab_name)) {
        throw TableNotFoundError(tab_name);
    }
    // erase table meta
    db_.WLatch();
    db_.tabs_.erase(tab_name);
    flush_meta();
    db_.WUnLatch();
    // destroy record file
    latch_.lock_shared();
    RmFileHandle* rh = fhs_[tab_name].get();
    latch_.unlock_shared();
    rm_manager_->close_file(rh);
    rm_manager_->destroy_file(tab_name);
    latch_.lock();
    fhs_.erase(tab_name);
    latch_.unlock();
}

/**
 * @description: 创建索引
 * @param {string&} tab_name 表的名称
 * @param {vector<string>&} col_names 索引包含的字段名称
 * @param {Context*} context
 */
void SmManager::create_index(const std::string& tab_name, const std::vector<std::string>& col_names, Context* context) {
    if (!db_.is_table(tab_name)) {
        throw TableNotFoundError(tab_name);
    }
    if (ix_manager_->exists(tab_name, col_names)) {
        throw IndexExistsError(tab_name, col_names);
    }
    std::vector<ColMeta> col_meta_vec;
    col_meta_vec.reserve(col_names.size());

    int col_tot_len = 0;
    int col_num = col_names.size();
    db_.RLatch();
    TabMeta& tab = db_.get_table(tab_name);
    for (auto& col_name : col_names) {
        auto col_meta = *(tab.get_col(col_name));
        col_tot_len += col_meta.len;
        col_meta_vec.push_back(col_meta);
    }
    db_.RUnLatch();

    if (col_tot_len > IX_MAX_COL_LEN) {
        throw InvalidColLengthError(col_tot_len);
    }

    IndexMeta idx = {
        .tab_name = tab_name,
        .col_tot_len = col_tot_len,
        .col_num = col_num,
        .cols = col_meta_vec,
    };
    db_.WLatch();
    db_.get_table(tab_name).indexes.push_back(idx);
    flush_meta();
    db_.WUnLatch();

    ix_manager_->create_index(tab_name, col_meta_vec);
    const std::string& index_name = ix_manager_->get_index_name(tab_name, col_names);
    std::unique_ptr<IxIndexHandle> idx_hdl_unptr = ix_manager_->open_index(index_name);
    IxIndexHandle* idx_hdl = idx_hdl_unptr.get();
    latch_.lock();
    ihs_.emplace(index_name, std::move(idx_hdl_unptr));
    latch_.unlock();
    // 全表扫描，加S锁
    if(context!=nullptr){
        context->lock_mgr_->lock_shared_on_table(context->txn_, disk_manager_->get_file_fd(tab_name));
    }
    latch_.lock_shared();
    RmFileHandle* fh = fhs_.at(tab_name).get();
    latch_.unlock_shared();
    ix_manager_->init_tree(idx_hdl, fh, col_meta_vec);
}

/**
 * @description: 删除索引
 * @param {string&} tab_name 表名称
 * @param {vector<string>&} col_names 索引包含的字段名称
 * @param {Context*} context
 */
void SmManager::drop_index(const std::string& tab_name, const std::vector<std::string>& col_names, Context* context) {
    if (!db_.is_table(tab_name)) {
        throw TableNotFoundError(tab_name);
    }
    const std::string& index_name = ix_manager_->get_index_name(tab_name, col_names);
    if (!disk_manager_->is_file(index_name)) {
        throw FileNotFoundError(index_name);
    }

    db_.WLatch();
    std::vector<IndexMeta>& table_indexes = db_.get_table(tab_name).indexes;
    bool found = false;
    for (auto iter = table_indexes.begin(); iter != table_indexes.end(); ++iter) {
        std::vector<ColMeta>& col_metas = (*iter).cols;
        if (col_names.size() == col_metas.size()) {
            size_t i;
            for (i = 0; i < col_metas.size(); ++i) {
                if (col_metas[i].name != col_names[i]) {
                    break;
                }
            }
            if (i == col_metas.size()) {
                found = true;
                table_indexes.erase(iter);
                break;
            }
        }
    }
    flush_meta();
    db_.WUnLatch();

    if (!found) {
        throw IndexNotFoundError(tab_name, col_names);
    }
    latch_.lock_shared();
    IxIndexHandle* idx_hdl = ihs_[index_name].get();
    latch_.unlock_shared();
    ix_manager_->close_index(idx_hdl);
    // 没有调用ix_manager->destroy_index,减少再次得到index_name的花销
    disk_manager_->destroy_file(index_name);
    latch_.lock();
    ihs_.erase(index_name);
    latch_.unlock();
}

/**
 * @description: 删除索引
 * @param {string&} tab_name 表名称
 * @param {vector<ColMeta>&} 索引包含的字段元数据
 * @param {Context*} context
 */
void SmManager::drop_index(const std::string& tab_name, const std::vector<ColMeta>& cols, Context* context) {
    std::vector<std::string> col_names;
    for (auto& colmeta : cols) {
        col_names.emplace_back(colmeta.name);
    }
    drop_index(tab_name, col_names, context);
}
