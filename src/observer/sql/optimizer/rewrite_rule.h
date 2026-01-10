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

#include "common/sys/rc.h"
#include "common/lang/memory.h"

class LogicalOperator;
class Expression;

/**
 * @brief 查询重写规则基类
 * @file rewrite_rule.h
 * @ingroup Optimizer
 * 
 * 该文件定义了两种重写规则的基类：
 * 1. RewriteRule：用于重写整个逻辑执行计划
 * 2. ExpressionRewriteRule：用于重写表达式
 * 
 * 这些规则是优化器的重要组成部分，用于在生成物理计划之前对逻辑计划或表达式进行转换，
 * 以提高查询执行效率。
 * 
 * @todo 重构当前的查询改写规则，将其整合到级联优化器（cascade optimizer）中。
 */

/**
 * @brief 逻辑计划重写规则基类
 * @ingroup Optimizer
 * 
 * RewriteRule是所有逻辑计划重写规则的抽象基类。它定义了一个统一的接口，
 * 用于对逻辑执行计划进行重写操作。每个具体的重写规则都需要继承这个类并实现
 * rewrite方法。
 */
class RewriteRule
{
public:
  /**
   * @brief 虚析构函数
   */
  virtual ~RewriteRule() = default;

  /**
   * @brief 重写逻辑执行计划
   * 
   * @param oper 要重写的逻辑操作符
   * @param change_made 输出参数，表示是否对逻辑计划进行了修改
   * @return RC 返回状态码，SUCCESS表示重写成功，其他值表示重写失败
   */
  virtual RC rewrite(unique_ptr<LogicalOperator> &oper, bool &change_made) = 0;
};

/**
 * @brief 表达式重写规则基类
 * @ingroup Optimizer
 * 
 * ExpressionRewriteRule是所有表达式重写规则的抽象基类。它定义了一个统一的接口，
 * 用于对表达式进行重写操作。每个具体的表达式重写规则都需要继承这个类并实现
 * rewrite方法。
 */
class ExpressionRewriteRule
{
public:
  /**
   * @brief 虚析构函数
   */
  virtual ~ExpressionRewriteRule() = default;

  /**
   * @brief 重写表达式
   * 
   * @param expr 要重写的表达式
   * @param change_made 输出参数，表示是否对表达式进行了修改
   * @return RC 返回状态码，SUCCESS表示重写成功，其他值表示重写失败
   */
  virtual RC rewrite(unique_ptr<Expression> &expr, bool &change_made) = 0;
};
