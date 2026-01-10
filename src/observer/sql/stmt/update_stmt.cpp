/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/5/22.
//

#include "sql/stmt/update_stmt.h"

UpdateStmt::UpdateStmt(Table *table, Value *values, int value_amount)
    : table_(table), values_(values), value_amount_(value_amount)
{}

#include "sql/stmt/update_stmt.h"
#include "common/log/log.h"
#include "sql/stmt/filter_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"

UpdateStmt::UpdateStmt(Table *table, Value *values, int value_amount)
    : table_(table), values_(values), value_amount_(value_amount)
{}

UpdateStmt::UpdateStmt(Table *table, const string &attribute_name, Value *values, int value_amount, FilterStmt *filter_stmt)
    : table_(table), attribute_name_(attribute_name), values_(values), value_amount_(value_amount), filter_stmt_(filter_stmt)
{}

UpdateStmt::~UpdateStmt()
{
  if (nullptr != values_) {
    delete[] values_;
    values_ = nullptr;
  }
  if (nullptr != filter_stmt_) {
    delete filter_stmt_;
    filter_stmt_ = nullptr;
  }
}

RC UpdateStmt::create(Db *db, const UpdateSqlNode &update, Stmt *&stmt)
{
  const char *table_name = update.relation_name.c_str();
  if (nullptr == db || nullptr == table_name) {
    LOG_WARN("invalid argument. db=%p, table_name=%p", db, table_name);
    return RC::INVALID_ARGUMENT;
  }

  // check whether the table exists
  Table *table = db->find_table(table_name);
  if (nullptr == table) {
    LOG_WARN("no such table. db=%s, table_name=%s", db->name(), table_name);
    return RC::SCHEMA_TABLE_NOT_EXIST;
  }

  // check whether the attribute exists
  const char *attr_name = update.attribute_name.c_str();
  const TableMeta &table_meta = table->table_meta();
  const FieldMeta *field_meta = table_meta.field(attr_name);
  if (nullptr == field_meta) {
    LOG_WARN("no such field. table=%s, field=%s", table_name, attr_name);
    return RC::SCHEMA_FIELD_NOT_EXIST;
  }

  // check whether the value type matches the field type
  const Value &value = update.value;
  if (value.attr_type() != field_meta->type()) {
    LOG_WARN("value type mismatch. table=%s, field=%s, expected=%d, actual=%d", 
             table_name, attr_name, field_meta->type(), value.attr_type());
    return RC::SCHEMA_FIELD_TYPE_MISMATCH;
  }

  // create filter statement if conditions exist
  FilterStmt *filter_stmt = nullptr;
  if (!update.conditions.empty()) {
    unordered_map<string, Table *> table_map;
    table_map.insert(pair<string, Table *>(string(table_name), table));

    RC rc = FilterStmt::create(
        db, table, &table_map, update.conditions.data(), static_cast<int>(update.conditions.size()), filter_stmt);
    if (rc != RC::SUCCESS) {
      LOG_WARN("failed to create filter statement. rc=%d:%s", rc, strrc(rc));
      return rc;
    }
  }

  // create values array for update
  Value *values = new Value[1];
  values[0] = update.value;

  stmt = new UpdateStmt(table, update.attribute_name, values, 1, filter_stmt);
  return RC::SUCCESS;
}
