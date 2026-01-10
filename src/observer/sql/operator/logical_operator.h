/**
 * @file logical_operator.h
 * @brief SQL逻辑操作符的核心头文件
 * 
 * 该文件定义了SQL逻辑操作符的抽象基类LogicalOperator以及逻辑操作符类型枚举。
 * 逻辑操作符描述了执行计划要做什么，而物理操作符则描述了如何做。
 * 
 * @author Wangyunlai
 * @date 2022年12月7日
 * @version 1.0
 */

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

#include "sql/expr/expression.h"           // 表达式定义
#include "sql/operator/operator_node.h"     // 操作符节点基类
#include "common/lang/unordered_set.h"      // 无序集合定义

/**
 * @defgroup LogicalOperator 逻辑操作符
 * @brief SQL执行计划的逻辑描述
 * 
 * 逻辑操作符描述了当前执行计划要做什么，比如从表中获取数据、过滤、投影、连接等等。
 * 与物理操作符不同，逻辑操作符不关心如何执行，只关心执行什么。
 * 
 * 逻辑操作符是优化器的输入，优化器会根据逻辑操作符生成最优的物理操作符执行计划。
 */

/**
 * @brief 逻辑算子
 * @defgroup LogicalOperator
 * @details 逻辑算子描述当前执行计划要做什么，比如从表中获取数据，过滤，投影，连接等等。
 * 物理算子会描述怎么做某件事情，这是与其不同的地方。
 */

/**
 * @brief 逻辑操作符类型枚举
 * @ingroup LogicalOperator
 * 
 * 该枚举定义了系统支持的所有逻辑操作符类型，每种类型对应一种逻辑操作。
 */
enum class LogicalOperatorType
{
  CALC,        ///< 计算操作符，用于执行计算表达式
  TABLE_GET,   ///< 表获取操作符，从表中获取数据
  PREDICATE,   ///< 谓词操作符，用于过滤数据
  PROJECTION,  ///< 投影操作符，对应SELECT语句中的列选择
  JOIN,        ///< 连接操作符，用于连接多个表
  INSERT,      ///< 插入操作符，用于插入数据
  DELETE,      ///< 删除操作符，用于删除数据
  UPDATE,      ///< 更新操作符，用于更新数据
  EXPLAIN,     ///< 解释操作符，用于查看执行计划
  GROUP_BY,    ///< 分组操作符，用于GROUP BY子句
};

/**
 * @brief 逻辑操作符基类
 * @ingroup LogicalOperator
 * 
 * 逻辑操作符描述了当前执行计划要做什么，比如从表中获取数据、过滤、投影、连接等等。
 * 逻辑操作符是优化器的输入，优化器会根据逻辑操作符生成最优的物理操作符执行计划。
 * 
 * @see OptimizeStage 查看逻辑操作符如何被优化
 */
class LogicalOperator : public OperatorNode
{
public:
  /**
   * @brief 默认构造函数
   */
  LogicalOperator() = default;

  /**
   * @brief 虚析构函数，确保子类析构函数被正确调用
   */
  virtual ~LogicalOperator();

  /**
   * @brief 获取逻辑操作符类型
   * @return 逻辑操作符类型
   */
  virtual LogicalOperatorType type() const = 0;

  /**
   * @brief 判断是否为物理操作符
   * @return 总是返回false
   */
  bool is_physical() const override { return false; }

  /**
   * @brief 判断是否为逻辑操作符
   * @return 总是返回true
   */
  bool is_logical() const override { return true; }

  /**
   * @brief 添加子操作符
   * @param oper 子操作符的智能指针
   */
  void add_child(unique_ptr<LogicalOperator> oper);

  /**
   * @brief 添加表达式
   * @param expr 表达式的智能指针
   */
  void add_expressions(unique_ptr<Expression> expr);

  /**
   * @brief 获取子操作符列表
   * @return 子操作符列表的引用
   */
  auto children() -> vector<unique_ptr<LogicalOperator>> & { return children_; }

  /**
   * @brief 获取表达式列表
   * @return 表达式列表的引用
   */
  auto expressions() -> vector<unique_ptr<Expression>> & { return expressions_; }

  /**
   * @brief 判断是否可以生成向量化操作符
   * @param type 逻辑操作符类型
   * @return 是否可以生成向量化操作符
   */
  static bool can_generate_vectorized_operator(const LogicalOperatorType &type);

  /**
   * @brief 生成通用子节点
   * @details 用于级联优化器的临时函数，需要移除
   */
  void generate_general_child();

protected:
  vector<unique_ptr<LogicalOperator>> children_;  ///< 子操作符列表

  /**
   * @brief 表达式列表
   * 
   * 表达式用于表示各种SQL操作，比如select中的列、where中的谓词、group by中的表达式等。
   * 表达式可以是常量、函数、列引用、子查询等。
   */
  vector<unique_ptr<Expression>> expressions_;
};
