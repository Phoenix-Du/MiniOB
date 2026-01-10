/**
 * @file logical_plan_generator.h
 * @brief SQL逻辑计划生成器的核心头文件
 * 
 * 该文件定义了LogicalPlanGenerator类，负责将SQL语句转换为逻辑执行计划。
 * 逻辑计划描述了SQL语句要做什么，比如从表中获取数据、过滤、投影、连接等。
 * 
 * @author Wangyunlai
 * @date 2023年8月16日
 * @version 1.0
 */

/* Copyright (c) 2023 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "common/lang/memory.h"       // 内存管理
#include "common/sys/rc.h"            // 状态码定义
#include "common/type/attr_type.h"    // 属性类型定义

class Stmt;           // 前向声明语句基类
class CalcStmt;       // 前向声明计算语句类
class SelectStmt;     // 前向声明选择语句类
class FilterStmt;     // 前向声明过滤语句类
class InsertStmt;     // 前向声明插入语句类
class DeleteStmt;     // 前向声明删除语句类
class UpdateStmt;     // 前向声明更新语句类
class ExplainStmt;    // 前向声明解释语句类
class LogicalOperator;// 前向声明逻辑操作符类

/**
 * @defgroup Optimizer 优化器模块
 * @brief SQL语句的优化和执行计划生成
 * 
 * 优化器模块负责将解析后的SQL语句转换为高效的执行计划，包括逻辑计划生成、
 * 计划重写和物理计划生成等功能。
 */

/**
 * @brief 逻辑计划生成器类
 * @ingroup Optimizer
 * 
 * LogicalPlanGenerator负责将SQL语句转换为逻辑执行计划。
 * 它根据不同类型的SQL语句（SELECT、INSERT、DELETE等）生成相应的逻辑操作符树。
 */
class LogicalPlanGenerator
{
public:
  /**
   * @brief 默认构造函数
   */
  LogicalPlanGenerator()          = default;

  /**
   * @brief 虚析构函数
   */
  virtual ~LogicalPlanGenerator() = default;

  /**
   * @brief 为SQL语句创建逻辑执行计划
   * 
   * 该方法是逻辑计划生成器的入口点，根据SQL语句的类型调用相应的创建方法。
   * 
   * @param stmt SQL语句对象
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create(Stmt *stmt, unique_ptr<LogicalOperator> &logical_operator);

private:
  /**
   * @brief 为计算语句创建逻辑执行计划
   * 
   * @param calc_stmt 计算语句对象
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(CalcStmt *calc_stmt, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 为选择语句创建逻辑执行计划
   * 
   * @param select_stmt 选择语句对象
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 为过滤语句创建逻辑执行计划
   * 
   * @param filter_stmt 过滤语句对象
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(FilterStmt *filter_stmt, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 为插入语句创建逻辑执行计划
   * 
   * @param insert_stmt 插入语句对象
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(InsertStmt *insert_stmt, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 为删除语句创建逻辑执行计划
   * 
   * @param delete_stmt 删除语句对象
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(DeleteStmt *delete_stmt, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 为更新语句创建逻辑执行计划
   * 
   * @param update_stmt 更新语句对象
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(UpdateStmt *update_stmt, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 为解释语句创建逻辑执行计划
   * 
   * @param explain_stmt 解释语句对象
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(ExplainStmt *explain_stmt, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 为带有分组的选择语句创建逻辑执行计划
   * 
   * @param select_stmt 选择语句对象，包含分组信息
   * @param logical_operator 输出参数，生成的逻辑执行计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_group_by_plan(SelectStmt *select_stmt, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 计算隐式类型转换的代价
   * 
   * @param from 源数据类型
   * @param to 目标数据类型
   * @return int 类型转换的代价
   */
  int implicit_cast_cost(AttrType from, AttrType to);
};
