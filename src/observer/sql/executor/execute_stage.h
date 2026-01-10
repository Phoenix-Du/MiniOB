/**
 * @file execute_stage.h
 * @brief SQL执行阶段的核心头文件
 * 
 * 该文件定义了SQL执行阶段的核心类ExecuteStage，负责SQL语句的执行。
 * ExecuteStage是SQL处理流程中的最后一个阶段，负责根据前面阶段生成的结果执行SQL语句。
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

#include "common/sys/rc.h"  // 状态码定义

class SQLStageEvent;  // 前向声明SQL阶段事件类
class SessionEvent;   // 前向声明会话事件类
class SelectStmt;     // 前向声明选择语句类

/**
 * @defgroup SQLStage SQL阶段
 * @brief SQL处理流程中的各个阶段
 * 
 * 这个模块定义了SQL处理流程中的各个阶段，包括解析、验证、优化和执行等阶段。
 */

/**
 * @brief SQL执行阶段的核心类
 * @ingroup SQLStage
 * @details 执行SQL语句的Stage，包括DML（数据操作语言）和DDL（数据定义语言）语句。
 * 根据前面阶段生成的结果，有些语句会生成执行计划，有些不会。整体上分为两类：
 * 1. 带执行计划的语句（如SELECT、INSERT、UPDATE、DELETE等）
 * 2. CommandExecutor可以直接执行的语句（如CREATE TABLE、DROP TABLE等）
 */
class ExecuteStage
{
public:
  /**
   * @brief 处理SQL请求的入口方法
   * 
   * 根据SQL语句的类型，选择合适的执行方式。对于需要生成执行计划的语句，
   * 会调用handle_request_with_physical_operator方法；对于不需要生成执行计划的语句，
   * 会直接调用对应的CommandExecutor执行。
   * 
   * @param event SQL阶段事件，包含了SQL请求的所有信息
   * @return RC 返回状态码，SUCCESS表示执行成功，其他值表示执行失败
   */
  RC handle_request(SQLStageEvent *event);
  
  /**
   * @brief 处理带有物理操作符的SQL请求
   * 
   * 对于需要生成执行计划的语句，该方法会执行物理操作符树，完成SQL语句的执行。
   * 
   * @param sql_event SQL阶段事件，包含了SQL请求的所有信息
   * @return RC 返回状态码，SUCCESS表示执行成功，其他值表示执行失败
   */
  RC handle_request_with_physical_operator(SQLStageEvent *sql_event);
};","}}}
