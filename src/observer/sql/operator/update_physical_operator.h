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

#include "sql/operator/physical_operator.h"
#include "storage/table/table.h"
#include "sql/parser/parse_defs.h"

/**
 * @brief 更新物理操作符
 * @ingroup PhysicalOperator
 * 
 * UpdatePhysicalOperator用于执行UPDATE语句的物理操作，实现记录的更新功能。
 */
class UpdatePhysicalOperator : public PhysicalOperator
{
public:
  /**
   * @brief 构造函数
   * @param table 要更新的表指针
   * @param field_names 要更新的字段名列表
   * @param values 要更新的值列表
   */
  UpdatePhysicalOperator(Table *table, const std::vector<std::string> &field_names, const std::vector<Value> &values);
  virtual ~UpdatePhysicalOperator();

  PhysicalOperatorType type() const override;
  RC open(Trx *trx) override;
  RC next() override;
  RC close() override;
  Tuple *current_tuple() override;

private:
  RC update_record(Record &record);

private:
  Table *table_ = nullptr;                    ///< 要更新的表指针
  std::vector<std::string> field_names_;      ///< 要更新的字段名列表
  std::vector<Value> values_;                 ///< 要更新的值列表
  Trx *trx_ = nullptr;                        ///< 事务上下文
};