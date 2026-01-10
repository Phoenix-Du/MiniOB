/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "sql/operator/logical_operator.h"
#include "storage/table/table.h"
#include "sql/parser/parse_defs.h"

/**
 * @brief 更新逻辑操作符
 * @ingroup LogicalOperator
 * 
 * UpdateLogicalOperator用于执行UPDATE语句的逻辑操作，生成更新逻辑计划。
 */
class UpdateLogicalOperator : public LogicalOperator
{
public:
  /**
   * @brief 构造函数
   * @param table 要更新的表指针
   * @param attribute_name 要更新的字段名
   * @param values 要更新的值列表
   * @param value_amount 值的数量
   */
  UpdateLogicalOperator(Table *table, const char *attribute_name, Value *values, int value_amount);
  virtual ~UpdateLogicalOperator();

  LogicalOperatorType type() const override;

public:
  Table *table() const { return table_; }
  const char *attribute_name() const { return attribute_name_.c_str(); }
  Value *values() const { return values_; }
  int value_amount() const { return value_amount_; }

private:
  Table *table_ = nullptr;                ///< 要更新的表指针
  std::string attribute_name_;            ///< 要更新的字段名
  Value *values_ = nullptr;               ///< 要更新的值列表
  int value_amount_ = 0;                  ///< 值的数量
};