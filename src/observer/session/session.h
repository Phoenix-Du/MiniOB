/**
 * @file session.h
 * @brief 会话管理的核心定义
 * 
 * 该文件定义了Session类，用于管理数据库会话。一个会话代表了客户端与服务器之间的一次连接，
 * 包含了当前连接的数据库、事务状态、执行参数等信息。
 * 
 * @author Wangyunlai
 * @date 2021/5/12
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

#include "common/types.h"
#include "common/lang/string.h"

class Trx;
class Db;
class SessionEvent;

/**
 * @defgroup Session 会话管理模块
 * @brief 负责管理客户端与服务器之间的会话
 * 
 * 会话管理模块处理客户端连接的创建、维护和销毁，以及会话相关的状态管理。
 * 每个客户端连接对应一个会话对象，会话对象包含了当前连接的数据库、事务状态、执行参数等信息。
 */

/**
 * @brief 会话类
 * @ingroup Session
 * 
 * 表示客户端与服务器之间的一次连接会话。当前实现中，一个连接对应一个会话，
 * 没有做特殊的会话管理，这简化了会话处理逻辑。
 */
class Session
{
public:
  /**
   * @brief 获取默认的会话对象
   * 
   * 新生成的会话都基于默认会话设置参数。
   * 
   * @note 当前并没有会话参数需要继承
   * @return Session& 默认会话对象的引用
   */
  static Session &default_session();

public:
  /**
   * @brief 默认构造函数
   */
  Session() = default;
  
  /**
   * @brief 析构函数
   */
  ~Session();

  /**
   * @brief 拷贝构造函数
   * 
   * @param other 要拷贝的会话对象
   */
  Session(const Session &other);
  
  /**
   * @brief 禁用赋值操作符
   */
  void operator=(Session &) = delete;

  /**
   * @brief 获取当前数据库名称
   * 
   * @return const char* 当前数据库名称
   */
  const char *get_current_db_name() const;
  
  /**
   * @brief 获取当前数据库对象
   * 
   * @return Db* 当前数据库对象
   */
  Db         *get_current_db() const;

  /**
   * @brief 设置当前会话关联的数据库
   * 
   * @param dbname 数据库名字
   */
  void set_current_db(const string &dbname);

  /**
   * @brief 设置当前事务为多语句模式
   * 
   * 在多语句模式下，事务需要明确的提交或回滚操作。
   * 
   * @param multi_operation_mode 是否为多语句模式
   */
  void set_trx_multi_operation_mode(bool multi_operation_mode);

  /**
   * @brief 检查当前事务是否为多语句模式
   * 
   * @return bool 如果是多语句模式返回true，否则返回false
   */
  bool is_trx_multi_operation_mode() const;

  /**
   * @brief 获取当前会话关联的事务
   * 
   * 如果当前没有事务，会创建一个新的事务。
   * 
   * @return Trx* 当前事务对象
   */
  Trx *current_trx();

  /**
   * @brief 销毁当前事务
   * 
   * 结束当前事务并释放相关资源。
   */
  void destroy_trx();

  /**
   * @brief 设置当前正在处理的请求
   * 
   * @param request 当前正在处理的请求对象
   */
  void set_current_request(SessionEvent *request);

  /**
   * @brief 获取当前正在处理的请求
   * 
   * @return SessionEvent* 当前正在处理的请求对象
   */
  SessionEvent *current_request() const;

  /**
   * @brief 设置SQL调试模式
   * 
   * @param sql_debug 是否启用SQL调试
   */
  void set_sql_debug(bool sql_debug) { sql_debug_ = sql_debug; }
  
  /**
   * @brief 检查是否启用了SQL调试模式
   * 
   * @return bool 如果启用了SQL调试返回true，否则返回false
   */
  bool sql_debug_on() const { return sql_debug_; }

  /**
   * @brief 设置是否使用哈希连接
   * 
   * @param hash_join 是否使用哈希连接
   */
  void set_hash_join(bool hash_join) { hash_join_ = hash_join; }
  
  /**
   * @brief 检查是否使用哈希连接
   * 
   * @return bool 如果使用哈希连接返回true，否则返回false
   */
  bool hash_join_on() const { return hash_join_; }

  /**
   * @brief 设置是否使用级联模式
   * 
   * @param use_cascade 是否使用级联模式
   */
  void set_use_cascade(bool use_cascade) { use_cascade_ = use_cascade; }
  
  /**
   * @brief 检查是否使用级联模式
   * 
   * @return bool 如果使用级联模式返回true，否则返回false
   */
  bool use_cascade() const { return use_cascade_; }

  /**
   * @brief 设置执行模式
   * 
   * @param mode 执行模式
   */
  void          set_execution_mode(const ExecutionMode mode) { execution_mode_ = mode; }
  
  /**
   * @brief 获取执行模式
   * 
   * @return ExecutionMode 当前执行模式
   */
  ExecutionMode get_execution_mode() const { return execution_mode_; }

  /**
   * @brief 检查是否使用了块模式
   * 
   * @return bool 如果使用了块模式返回true，否则返回false
   */
  bool used_chunk_mode() { return used_chunk_mode_; }

  /**
   * @brief 设置是否使用块模式
   * 
   * @param used_chunk_mode 是否使用块模式
   */
  void set_used_chunk_mode(bool used_chunk_mode) { used_chunk_mode_ = used_chunk_mode; }

  /**
   * @brief 将指定会话设置到线程变量中
   *
   */
  static void set_current_session(Session *session);

  /**
   * @brief 获取当前的会话
   * @details 当前某个请求开始时，会将会话设置到线程变量中，在整个请求处理过程中不会改变
   */
  static Session *current_session();

private:
  Db           *db_              = nullptr;
  Trx          *trx_             = nullptr;
  SessionEvent *current_request_ = nullptr;  ///< 当前正在处理的请求

  bool trx_multi_operation_mode_ = false;  ///< 当前事务的模式，是否多语句模式. 单语句模式自动提交

  bool sql_debug_   = false;  ///< 是否输出SQL调试信息
  bool hash_join_   = false;  ///< 是否使用hash join
  bool use_cascade_ = false;  ///< 是否使用 cascade 优化器

  // 是否使用了 `chunk_iterator` 模式。 只有在设置了 `chunk_iterator`
  // 并且可以生成相关物理执行计划时才会使用 `chunk_iterator` 模式。
  bool used_chunk_mode_ = false;

  ExecutionMode execution_mode_ = ExecutionMode::TUPLE_ITERATOR;
};
