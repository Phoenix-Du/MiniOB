/**
 * @file optimize_stage.h
 * @brief SQL优化阶段的核心头文件
 * 
 * 该文件定义了OptimizeStage类，负责将解析后的SQL语句转换为执行计划并进行优化。
 * 优化过程包括逻辑计划生成、计划重写和物理计划生成三个主要步骤。
 * 
 * @author Longda
 * @date 2021年4月13日
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

#include "common/sys/rc.h"                          // 状态码定义
#include "session/session.h"                        // 会话定义
#include "sql/operator/logical_operator.h"          // 逻辑操作符定义
#include "sql/operator/physical_operator.h"         // 物理操作符定义
#include "sql/optimizer/logical_plan_generator.h"   // 逻辑计划生成器
#include "sql/optimizer/physical_plan_generator.h"  // 物理计划生成器
#include "sql/optimizer/rewriter.h"                 // 计划重写器

class SQLStageEvent;    // 前向声明SQL阶段事件类
class LogicalOperator;  // 前向声明逻辑操作符类
class Stmt;             // 前向声明语句类

/**
 * @defgroup SQLStage SQL处理阶段
 * @brief SQL语句的处理流程
 * 
 * SQL语句在数据库中的处理分为多个阶段，包括解析、优化和执行等。
 * OptimizeStage是其中的优化阶段，负责将解析后的语句转换为可执行的物理计划。
 */

/**
 * @brief SQL优化阶段类
 * @ingroup SQLStage
 * 
 * OptimizeStage负责将解析后的SQL语句转换为执行计划并进行优化。
 * 优化过程包括：
 * 1. 将SQL语句转换为逻辑计划
 * 2. 根据规则重写逻辑计划
 * 3. 根据逻辑计划生成物理计划
 * 
 * 并不是所有的SQL语句都需要生成执行计划，比如DDL语句（CREATE TABLE、CREATE INDEX等）
 * 可以直接通过CommandExecutor执行。
 */
class OptimizeStage
{
public:
  /**
   * @brief 处理SQL请求，执行优化流程
   * 
   * 该方法是优化阶段的入口点，接收SQLStageEvent对象作为参数，
   * 并执行完整的优化流程：逻辑计划生成、计划重写、物理计划生成。
   * 
   * @param event SQL阶段事件，包含解析后的SQL语句和执行上下文
   * @return RC 返回状态码，SUCCESS表示优化成功，其他值表示优化失败
   */
  RC handle_request(SQLStageEvent *event);

private:
  /**
   * @brief 根据SQL生成逻辑计划
   * 
   * 逻辑计划描述了SQL语句要做什么，比如从表中获取数据、过滤、投影等。
   * 由于SQL语句可能包含嵌套结构（如子查询），因此需要递归生成逻辑计划。
   * 
   * @param sql_event SQL阶段事件，包含解析后的SQL语句信息
   * @param logical_operator 输出参数，生成的逻辑计划
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_logical_plan(SQLStageEvent *sql_event, unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 重写逻辑计划
   * 
   * 根据各种优化规则对逻辑计划进行重写，比如消除多余的比较（如1!=0）、
   * 简化表达式等。计划重写是一个递归的过程，会处理逻辑计划树的每个节点。
   * 
   * @param logical_operator 要重写的逻辑计划
   * @return RC 返回状态码，SUCCESS表示重写成功，其他值表示重写失败
   */
  RC rewrite(unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 优化逻辑计划
   * 
   * 当前该方法未实现具体功能。未来可以添加代价模型，根据代价选择最优的
   * 逻辑计划。
   * 
   * @param logical_operator 需要优化的逻辑计划
   * @return RC 返回状态码，当前固定返回SUCCESS
   */
  RC optimize(unique_ptr<LogicalOperator> &logical_operator);

  /**
   * @brief 根据逻辑计划生成物理计划
   * 
   * 物理计划描述了如何执行SQL语句，比如如何从表中获取数据、是否使用索引、
   * 使用哪种连接算法等。物理计划是一个多叉树结构，执行器会按照这个树结构
   * 实际执行SQL语句。
   * 
   * @param logical_operator 输入的逻辑计划
   * @param physical_operator 输出参数，生成的物理计划
   * @param session 会话对象，包含执行上下文信息
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC generate_physical_plan(
      unique_ptr<LogicalOperator> &logical_operator, 
      unique_ptr<PhysicalOperator> &physical_operator, 
      Session *session);

private:
  LogicalPlanGenerator  logical_plan_generator_;   ///< 逻辑计划生成器，负责将SQL转换为逻辑计划
  PhysicalPlanGenerator physical_plan_generator_;  ///< 物理计划生成器，负责将逻辑计划转换为物理计划
  Rewriter              rewriter_;                 ///< 计划重写器，负责根据规则重写逻辑计划
};
