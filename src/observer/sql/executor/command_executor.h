/**
 * @file command_executor.h
 * @brief SQL命令执行器的抽象基类
 * 
 * 该文件定义了CommandExecutor抽象类，作为所有SQL命令执行器的基类。
 * 一些SQL语句（如DDL语句）不会生成传统的执行计划，而是直接使用执行器来执行。
 * 
 * @author Wangyunlai
 * @date 2023/4/25
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

#include "common/sys/rc.h"

class SQLStageEvent;

/**
 * @defgroup Executor 执行器模块
 * @brief 负责SQL语句的执行，包括DDL、DML等各类语句
 * 
 * 执行器模块是SQL处理流程的最后一个阶段，负责将优化后的执行计划或直接将SQL命令转换为实际的数据库操作。
 * 对于一些简单的SQL语句（如DDL语句），不会生成完整的执行计划，而是直接使用对应的执行器来执行。
 */

/**
 * @brief 命令执行器抽象基类
 * @ingroup Executor
 * 
 * CommandExecutor是所有SQL命令执行器的基类，定义了执行SQL命令的统一接口。
 * 子类需要实现具体的execute方法来处理特定类型的SQL命令。
 * 
 * @note 该类采用模板方法模式设计，子类通过重写特定方法来实现不同命令的执行逻辑。
 */
class CommandExecutor
{
public:
  /**
   * @brief 默认构造函数
   */
  CommandExecutor()          = default;
  
  /**
   * @brief 虚析构函数，确保子类可以正确析构
   */
  virtual ~CommandExecutor() = default;

  /**
   * @brief 执行SQL命令
   * 
   * 该方法是执行SQL命令的入口点，接收SQLStageEvent对象作为参数，该对象包含了SQL语句的解析结果和上下文信息。
   * 
   * @param sql_event SQL阶段事件，包含SQL语句的解析结果和执行上下文
   * @return RC 返回状态码，SUCCESS表示执行成功，其他值表示执行失败
   */
  RC execute(SQLStageEvent *sql_event);

protected:
  /**
   * @brief 执行SQL命令的具体实现
   * 
   * 子类需要重写此方法来实现特定SQL命令的执行逻辑。
   * 
   * @param sql_event SQL阶段事件，包含SQL语句的解析结果和执行上下文
   * @return RC 返回状态码，SUCCESS表示执行成功，其他值表示执行失败
   */
  virtual RC do_execute(SQLStageEvent *sql_event) = 0;
};
