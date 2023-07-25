#pragma once
#include <cstring>
#include <memory>

#include "common/common.h"
#include "defs.h"
#include "errors.h"
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "record/rm_defs.h"
#include "system/sm.h"
#include "system/sm_meta.h"

class AggExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> prev_;  // 投影节点的儿子节点
    std::vector<ColMeta> cols_;               // 需要投影的字段
    ssize_t len_;                             // 字段总长度
    std::vector<ssize_t> sel_idxs_;
    ColMeta col_;
    RmRecord rec_;
    TabCol sel_col_;

    Value max_res_;
    Value min_res_;
    Value sum_res_;
    Value count_res_;

   public:
    AggExecutor(std::unique_ptr<AbstractExecutor> prev, TabCol &sel_col) {
        assert(sel_col.has_agg);
        prev_ = std::move(prev);
        // ssize_t curr_offset = 0;
        sel_col_ = sel_col;


        auto cols = prev_->cols();

        if (cols.size() != 1) {
            throw InternalError("error running");
        }
        col_ = cols[0];
        // auto col = *pos;
        // col.offset = curr_offset;
        // curr_offset += col.len;
        // cols_.push_back(col);
        // len_ = curr_offset;
        // rec_ = RmRecord(len_);
    }

    void beginTuple() override {
        switch (sel_col_.aggregate_type) {
            case AggType::MAX:
                beginMax();
                break;
            case AggType::MIN:
                beginMin();
                break;
            case AggType::SUM:
                beginSum();
                break;
            case AggType::COUNT:
                beginCount();
                break;
            default:
                throw InternalError("unsupported agg function");
        }
        if( sel_col_.aggregate_type== AggType::COUNT||sel_col_.aggregate_type== AggType::MAX||sel_col_.aggregate_type== AggType::MIN){
            bool type_match = col_.type==TYPE_INT || col_.type == TYPE_FLOAT || col_.type == TYPE_STRING;
            if(! type_match){
                throw InternalError("Agg function type not match");
            }
        }else if(sel_col_.aggregate_type == AggType::SUM){
            bool type_match = col_.type==TYPE_INT || col_.type == TYPE_FLOAT ;
            if(! type_match){
                throw InternalError("Agg function type not match");
            }
        }
    }
    void beginMax() {
        std::shared_ptr<RmRecord> rec;
        prev_->beginTuple();
        if(!prev_->is_end()){
            rec = prev_->Next();
            max_res_ = Value::read_from_record(rec, col_);
        }
        prev_->nextTuple();
        for ( ;!prev_->is_end(); prev_->nextTuple()) {
            rec = prev_->Next();
            auto val = Value::read_from_record(rec, col_);
            max_res_ = val > max_res_ ? val : max_res_;
        }
    }

    void beginMin() {
        std::shared_ptr<RmRecord> rec;
        prev_->beginTuple();
        if(!prev_->is_end()){
            rec = prev_->Next();
            min_res_ = Value::read_from_record(rec, col_);
        }
        prev_->nextTuple();
        for ( ;!prev_->is_end(); prev_->nextTuple()) {
            rec = prev_->Next();
            auto val = Value::read_from_record(rec, col_);
            min_res_ = val < min_res_ ? val : min_res_;
        }
    }
    void beginSum(){
        if(col_.type == TYPE_INT){
            sum_res_.set_int(0);

        }else if(col_.type == TYPE_FLOAT) {
            sum_res_.set_float(0.0);
        }
        else {
            throw InternalError("Agg function type not match");
        }

        for (prev_->beginTuple(); !prev_->is_end(); prev_->nextTuple()) {
            std::shared_ptr<RmRecord> rec = prev_->Next();

            auto val = Value::read_from_record(rec, col_);
            sum_res_ = sum_res_+ val;
        }
    };
    void beginCount(){



    };
    void nextTuple() override {
        if (!prev_->is_end()) {
            throw InternalError("should not be called");
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        auto ptr = std::make_unique<RmRecord>(len_);
        ptr->SetData(rec_.data);
        return ptr;
    }
    bool is_end() const override {
        bool ret = prev_->is_end();
        return ret;
    }

    size_t tupleLen() const override { return len_; };

    const std::vector<ColMeta> &cols() const override { return cols_; };

    ColMeta get_col_offset(const TabCol &target) override {
        for (auto &col : cols_) {
            if (col.tab_name == target.tab_name && col.name == target.col_name) {
                return col;
            }
        }
        throw ColumnNotFoundError(target.col_name);
    }
    std::string getType() override { return "ProjectionExecutor"; };
    Rid &rid() override { return _abstract_rid; }
};

// if(sel_cols[col_id].aggregate.get()!= NULL && sel_cols[col_id].aggregate->aggregate_type!= 4)
// {
//     has_aggregate = true;
// }
// std::string col_str;
// char *rec_buf = Tuple->data + col.offset;
// if (col.type == TYPE_INT) {
//     col_str = std::to_string(*(int *)rec_buf);
// } else if (col.type == TYPE_BIGINT) {
//     col_str = std::to_string(*(int64_t *)rec_buf);
// } else if (col.type == TYPE_FLOAT) {
//     col_str = std::to_string(*(float *)rec_buf);
// } else if (col.type == TYPE_STRING) {
//     col_str = std::string((char *)rec_buf, col.len);
//     col_str.resize(strlen(col_str.c_str()));
// } else if (col.type == TYPE_DATETIME) {
//     int64_t tmp = *(int64_t *)rec_buf;
//     const std::string &year =
//         std::to_string((tmp >> 40) & 0b1111111111111111);
//     const std::string &fyear =
//         std::string(4 - year.size(), '0') + year;
//
//     const std::string &month =
//         std::to_string((tmp >> 32) & 0b11111111);
//     const std::string &fmonth =
//         std::string(2 - month.size(), '0') + month;
//
//     const std::string &day =
//         std::to_string((tmp >> 24) & 0b11111111);
//     const std::string &fday =
//         std::string(2 - day.size(), '0') + day;
//
//     const std::string &hour =
//         std::to_string((tmp >> 16) & 0b11111111);
//     const std::string &fhour =
//         std::string(2 - hour.size(), '0') + hour;
//
//     const std::string &minute =
//         std::to_string((tmp >> 8) & 0b11111111);
//     const std::string &fminute =
//         std::string(2 - minute.size(), '0') + minute;
//
//     const std::string &sec = std::to_string(tmp & 0b11111111);
//     const std::string &fsec =
//         std::string(2 - sec.size(), '0') + sec;
//
//     // YYYY-MM-DD HH:MM:SS
//     col_str = fyear + "-" + fmonth + "-" + fday + " " + fhour +
//               ":" + fminute + ":" + fsec;
// }
// if(has_aggregate)
// {
//     // 处理聚集函数
//     switch(sel_cols[col_id].aggregate->aggregate_type){
//         case 0:
//             // COUNT
//             if(col.type == TYPE_INT || col.type == TYPE_FLOAT || col.type == TYPE_STRING)
//             {
//                 if(tmp_item.size() <= col_id){tmp_item.push_back("1");}
//                 else
//                 {
//                     tmp_item[col_id] = std::to_string(atoi(tmp_item[col_id].c_str()) + 1);
//                 }
//             }
//             else
//             {
//                 throw AggregateParamFormatError("COUNT");
//             }
//             break;
//         case 1:
//             // MAX
//             if(tmp_item.size() <= col_id){tmp_item.push_back(col_str);}
//             else
//             {
//                 if(col.type == TYPE_INT)
//                 {
//                     tmp_item[col_id] = atoi(tmp_item[col_id].c_str()) > atoi(col_str.c_str()) ?
//                     tmp_item[col_id]:col_str;
//                 }
//                 else if(col.type == TYPE_FLOAT)
//                 {
//                     tmp_item[col_id] = atof(tmp_item[col_id].c_str()) > atof(col_str.c_str()) ?
//                     tmp_item[col_id]:col_str;
//                 }
//                 else if(col.type == TYPE_STRING)
//                 {
//                     tmp_item[col_id] = tmp_item[col_id] > col_str ? tmp_item[col_id]:col_str;
//                 }
//                 else
//                 {
//                     throw AggregateParamFormatError("MAX");
//                 }
//             }
//             break;
//         case 2:
//             // MIN
//             if(tmp_item.size() <= col_id){tmp_item.push_back(col_str);}
//             else
//             {
//                 if(col.type == TYPE_INT)
//                 {
//                     tmp_item[col_id] = atoi(tmp_item[col_id].c_str()) < atoi(col_str.c_str()) ?
//                     tmp_item[col_id]:col_str;
//                 }
//                 else if(col.type == TYPE_FLOAT)
//                 {
//                     tmp_item[col_id] = atof(tmp_item[col_id].c_str()) < atof(col_str.c_str()) ?
//                     tmp_item[col_id]:col_str;
//                 }
//                 else if(col.type == TYPE_STRING)
//                 {
//                     tmp_item[col_id] = tmp_item[col_id] < col_str ? tmp_item[col_id]:col_str;
//                 }
//                 else
//                 {
//                     throw AggregateParamFormatError("MIN");
//                 }
//             }
//             break;
//         case 3:
//             // SUM
//             if(col.type == TYPE_INT)
//             {
//                 if(tmp_item.size() <= col_id){tmp_item.push_back(col_str);}
//                 else
//                 {
//                     tmp_item[col_id] = std::to_string(atoi(tmp_item[col_id].c_str()) + atoi(col_str.c_str()));
//                 }
//             }
//             else if(col.type == TYPE_FLOAT)
//             {
//                 if(tmp_item.size() <= col_id){tmp_item.push_back(col_str);}
//                 else
//                 {
//                     tmp_item[col_id] = std::to_string(atof(tmp_item[col_id].c_str()) + atof(col_str.c_str()));
//                 }
//             }
//             else
//             {
//                 throw AggregateParamFormatError("SUM");
//             }
//             break;
//         default: break;
//     }
// }
// else
// {
//     columns.push_back(col_str);
// }
// col_id++;
//
//
//  ---------------
// TabCol sel_col;
// sel_col.tab_name = sv_sel_col->tab_name;
// sel_col.col_name = sv_sel_col->col_name;
// sel_col.has_agg=false;
// if (sv_sel_col->aggregate_type != ast::AggregateType::NONE) {
//     sel_col.has_agg=true;
//     std::string tmp_col_name ;
//     sel_col.agg_arg_col_name=sel_col.col_name;
//     switch (sv_sel_col->aggregate_type) {
//         case ast::AggregateType::MAX:
//             sel_col.aggregate_type = AggType::MAX;
//             tmp_col_name = "MAX(";
//             tmp_col_name.append(sel_col.col_name);
//             tmp_col_name.append(")");
//             break;
//         case ast::AggregateType::COUNT:
//             sel_col.aggregate_type = AggType::COUNT;
//             tmp_col_name = "COUNT(";
//             tmp_col_name.append(sel_col.col_name);
//             tmp_col_name.append(")");
//             break;
//         case ast::AggregateType::MIN:
//             sel_col.aggregate_type = AggType::MIN;
//             tmp_col_name = "MIN(";
//             tmp_col_name.append(sel_col.col_name);
//             tmp_col_name.append(")");
//             break;
//         case ast::AggregateType::SUM:
//             sel_col.aggregate_type = AggType::SUM;
//             tmp_col_name = "SUM(";
//             tmp_col_name.append(sel_col.col_name);
//             tmp_col_name.append(")");
//             break;
//
//         default:
//             throw InternalError("unknown agg function");
//     }
//     if(sv_sel_col->another_name.size()!=0){
//         sel_col.col_name = sv_sel_col->another_name;
//     }else {
//         sel_col.col_name=tmp_col_name;
//     }
// }
// query->cols.push_back(sel_col);
