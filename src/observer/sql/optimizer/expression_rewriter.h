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
// Created by Wangyunlai on 2022/12/13.
//

#pragma once

#include "common/lang/memory.h"
#include "common/sys/rc.h"
#include "sql/expr/expression.h"
#include "sql/operator/logical_operator.h"
#include "sql/optimizer/rewrite_rule.h"

/**
 * @brief 表达式重写器
 * @file expression_rewriter.h
 * @ingroup Optimizer
 * 
 * 该文件定义了ExpressionRewriter类，用于对逻辑执行计划中的表达式进行重写。
 * ExpressionRewriter继承自RewriteRule，是一种特殊的重写规则，专门用于处理表达式的转换。
 */

/**
 * @brief 表达式重写器类
 * @ingroup Optimizer
 * 
 * ExpressionRewriter是一种特殊的重写规则，专门用于重写逻辑执行计划中的表达式。
 * 它遍历逻辑计划中的所有表达式，并应用一系列表达式重写规则对这些表达式进行转换，
 * 以提高查询执行效率。
 */
class ExpressionRewriter : public RewriteRule
{
public:
  /**
   * @brief 构造函数
   * 
   * 初始化表达式重写器并加载所有可用的表达式重写规则。
   */
  ExpressionRewriter();

  /**
   * @brief 虚析构函数
   */
  virtual ~ExpressionRewriter() = default;

  /**
   * @brief 重写逻辑执行计划
   * 
   * 遍历逻辑计划中的所有表达式，并应用表达式重写规则对它们进行转换。
   * 
   * @param oper 要重写的逻辑操作符
   * @param change_made 输出参数，表示是否对逻辑计划进行了修改
   * @return RC 返回状态码，SUCCESS表示重写成功，其他值表示重写失败
   */
  RC rewrite(unique_ptr<LogicalOperator> &oper, bool &change_made) override;

private:
  /**
   * @brief 重写单个表达式
   * 
   * 对给定的表达式应用所有表达式重写规则进行转换。
   * 
   * @param expr 要重写的表达式
   * @param change_made 输出参数，表示是否对表达式进行了修改
   * @return RC 返回状态码，SUCCESS表示重写成功，其他值表示重写失败
   */
  RC rewrite_expression(unique_ptr<Expression> &expr, bool &change_made);

private:
  /**
   * @brief 表达式重写规则列表
   * 
   * 存储所有可用的表达式重写规则。这些规则会在rewrite_expression方法中依次被应用。
   */
  vector<unique_ptr<ExpressionRewriteRule>> expr_rewrite_rules_;
};
