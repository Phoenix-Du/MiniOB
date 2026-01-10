/**
 * @file execute_stage.cpp
 * @brief SQL执行阶段的核心实现文件
 * 
 * 该文件实现了SQL执行阶段的核心类ExecuteStage的方法，负责SQL语句的执行。
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

#include "sql/executor/execute_stage.h"  // 自身头文件

#include "common/log/log.h"                      // 日志功能
#include "event/session_event.h"                 // 会话事件
#include "event/sql_event.h"                     // SQL事件
#include "sql/executor/command_executor.h"       // 命令执行器
#include "sql/operator/calc_physical_operator.h"  // 计算物理操作符
#include "sql/stmt/select_stmt.h"                // 选择语句
#include "sql/stmt/stmt.h"                       // SQL语句基类
#include "storage/default/default_handler.h"     // 默认存储处理器

using namespace common;  // 使用common命名空间

/**
 * @brief 处理SQL请求的入口方法
 * 
 * 根据SQL语句的类型，选择合适的执行方式：
 * 1. 如果有物理操作符（即执行计划），则调用handle_request_with_physical_operator方法执行
 * 2. 如果没有物理操作符，则调用CommandExecutor执行
 * 
 * @param sql_event SQL阶段事件，包含了SQL请求的所有信息
 * @return RC 返回状态码，SUCCESS表示执行成功，其他值表示执行失败
 */
RC ExecuteStage::handle_request(SQLStageEvent *sql_event)
{
  RC rc = RC::SUCCESS;  // 初始化返回状态码为成功

  // 获取物理操作符（执行计划）
  const unique_ptr<PhysicalOperator> &physical_operator = sql_event->physical_operator();
  if (physical_operator != nullptr) {
    // 如果有物理操作符，则调用handle_request_with_physical_operator方法执行
    return handle_request_with_physical_operator(sql_event);
  }

  // 获取会话事件
  SessionEvent *session_event = sql_event->session_event();

  // 获取SQL语句
  Stmt *stmt = sql_event->stmt();
  if (stmt != nullptr) {
    // 创建命令执行器并执行SQL语句
    CommandExecutor command_executor;
    rc = command_executor.execute(sql_event);
    // 设置执行结果的返回码
    session_event->sql_result()->set_return_code(rc);
  } else {
    // 如果stmt为null，则返回内部错误
    return RC::INTERNAL;
  }
  return rc;
}

/**
 * @brief 处理带有物理操作符的SQL请求
 * 
 * 对于需要生成执行计划的语句，该方法会将物理操作符设置到SqlResult中，
 * 后续由SqlResult负责执行物理操作符树并返回结果。
 * 
 * @param sql_event SQL阶段事件，包含了SQL请求的所有信息
 * @return RC 返回状态码，SUCCESS表示执行成功，其他值表示执行失败
 */
RC ExecuteStage::handle_request_with_physical_operator(SQLStageEvent *sql_event)
{
  RC rc = RC::SUCCESS;  // 初始化返回状态码为成功

  // 获取物理操作符（执行计划）
  unique_ptr<PhysicalOperator> &physical_operator = sql_event->physical_operator();
  // 断言物理操作符不为null
  ASSERT(physical_operator != nullptr, "physical operator should not be null");

  // 获取SqlResult并设置物理操作符
  SqlResult *sql_result = sql_event->session_event()->sql_result();
  sql_result->set_operator(std::move(physical_operator));
  return rc;
}
