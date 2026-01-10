/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sql/operator/update_physical_operator.h"
#include "common/log/log.h"
#include "storage/table/table.h"
#include "sql/stmt/update_stmt.h"
#include "sql/parser/parse_defs.h"
#include "storage/record/record.h"
#include "storage/trx/trx.h"

UpdatePhysicalOperator::UpdatePhysicalOperator(Table *table, const std::vector<std::string> &field_names, const std::vector<Value> &values)
    : table_(table), field_names_(field_names), values_(values)
{}

UpdatePhysicalOperator::~UpdatePhysicalOperator()
{}

PhysicalOperatorType UpdatePhysicalOperator::type() const
{
  return PhysicalOperatorType::UPDATE;
}

RC UpdatePhysicalOperator::open(Trx *trx)
{
  if (children_.empty()) {
    LOG_WARN("update physical operator must have child operator");
    return RC::INTERNAL;  // 内部错误
  }

  trx_ = trx;
  return children_[0]->open(trx);
}

RC UpdatePhysicalOperator::next()
{
  RC rc = RC::SUCCESS;
  PhysicalOperator *child = children_[0];

  while (RC::SUCCESS == (rc = child->next())) {
    Tuple *tuple = child->current_tuple();
    if (tuple == nullptr) {
      LOG_WARN("update physical operator get current tuple failed");
      return RC::INTERNAL;  // 内部错误
    }

    RecordTuple *record_tuple = dynamic_cast<RecordTuple *>(tuple);
    if (record_tuple == nullptr) {
      LOG_WARN("update physical operator get record tuple failed");
      return RC::INTERNAL;  // 内部错误
    }

    Record &record = record_tuple->record();
    rc = update_record(record);
    if (rc != RC::SUCCESS) {
      return rc;
    }
  }

  return rc;
}

RC UpdatePhysicalOperator::close()
{
  RC rc = RC::SUCCESS;
  if (!children_.empty()) {
    rc = children_[0]->close();
  }
  return rc;
}

Tuple *UpdatePhysicalOperator::current_tuple()
{
  return nullptr;  // 更新操作不返回结果集
}

RC UpdatePhysicalOperator::update_record(Record &record)
{
  // 目前只支持更新单个字段
  if (field_names_.empty() || values_.empty() || field_names_.size() != values_.size()) {
    LOG_WARN("update field names and values size mismatch");
    return RC::INTERNAL;  // 内部错误
  }

  const char *old_record_data = record.data();
  int record_size = table_->table_meta().record_size();
  
  // 创建新的记录数据
  char *new_record_data = new char[record_size];
  if (new_record_data == nullptr) {
    LOG_WARN("alloc new record data failed");
    return RC::INTERNAL;  // 内部错误
  }
  
  // 复制原有记录数据
  memcpy(new_record_data, old_record_data, record_size);

  // 更新指定字段的值
  const char *field_name = field_names_[0].c_str();
  const FieldMeta *field_meta = table_->table_meta().field(field_name);
  if (field_meta == nullptr) {
    LOG_WARN("field %s not found", field_name);
    delete[] new_record_data;
    return RC::SCHEMA_FIELD_NOT_EXIST;  // 字段不存在
  }

  // 更新指定字段的值
  Value *value = &values_[0];  // 目前只支持更新单个字段
  RC rc = value->to_string(field_meta, new_record_data + field_meta->offset());
  if (rc != RC::SUCCESS) {
    LOG_WARN("convert value to string failed: %s", strrc(rc));
    delete[] new_record_data;
    return rc;
  }

  // 创建新的Record对象
  Record new_record;
  new_record.set_data_owner(new_record_data, record_size);
  new_record.set_rid(record.rid());

  // 使用事务更新记录
  rc = trx_->update_record(table_, record, new_record);
  if (rc != RC::SUCCESS) {
    LOG_WARN("update record failed: %s", strrc(rc));
    return rc;
  }

  return RC::SUCCESS;
}