/**
 * @file stmt.h
 * @brief SQL语句的基类定义
 * 
 * 该文件定义了所有SQL语句的抽象基类Stmt，以及SQL语句类型的枚举定义。
 * Stmt类是SQL解析结果的进一步抽象，用于后续的查询优化和执行。
 * 
 * @author Wangyunlai
 * @date 2022年5月22日
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

#include "common/sys/rc.h"           // 状态码定义
#include "sql/parser/parse_defs.h"    // SQL解析相关的数据结构

class Db;  // 前向声明数据库类

/**
 * @defgroup Statement SQL语句模块
 * @brief SQL语句的定义和处理
 * 
 * 这个模块定义了各种SQL语句的类结构，包括基类Stmt和各种具体的SQL语句类。
 * SQL语句是SQL解析结果的进一步抽象，用于后续的查询优化和执行。
 */

/**
 * @brief Statement SQL语句解析后通过Resolver转换成Stmt
 * @defgroup Statement
 * @file stmt.h
 */

/**
 * @brief SQL语句的类型枚举
 * 
 * 定义了系统支持的所有SQL语句类型，包括查询、修改、DDL等操作。
 */
#define DEFINE_ENUM()             \
  DEFINE_ENUM_ITEM(CALC)          \
  DEFINE_ENUM_ITEM(SELECT)        \
  DEFINE_ENUM_ITEM(INSERT)        \
  DEFINE_ENUM_ITEM(UPDATE)        \
  DEFINE_ENUM_ITEM(DELETE)        \
  DEFINE_ENUM_ITEM(CREATE_TABLE)  \
  DEFINE_ENUM_ITEM(DROP_TABLE)    \
  DEFINE_ENUM_ITEM(ANALYZE_TABLE) \
  DEFINE_ENUM_ITEM(CREATE_INDEX)  \
  DEFINE_ENUM_ITEM(DROP_INDEX)    \
  DEFINE_ENUM_ITEM(SYNC)          \
  DEFINE_ENUM_ITEM(SHOW_TABLES)   \
  DEFINE_ENUM_ITEM(DESC_TABLE)    \
  DEFINE_ENUM_ITEM(BEGIN)         \
  DEFINE_ENUM_ITEM(COMMIT)        \
  DEFINE_ENUM_ITEM(ROLLBACK)      \
  DEFINE_ENUM_ITEM(LOAD_DATA)     \
  DEFINE_ENUM_ITEM(HELP)          \
  DEFINE_ENUM_ITEM(EXIT)          \
  DEFINE_ENUM_ITEM(EXPLAIN)       \
  DEFINE_ENUM_ITEM(PREDICATE)     \
  DEFINE_ENUM_ITEM(SET_VARIABLE)

/**
 * @brief SQL语句类型枚举类
 * 
 * 使用宏定义的方式生成所有的语句类型，方便统一管理和扩展。
 */
enum class StmtType
{
#define DEFINE_ENUM_ITEM(name) name,  ///< 生成枚举项
  DEFINE_ENUM()
#undef DEFINE_ENUM_ITEM
};

/**
 * @brief 获取SQL语句类型的字符串表示
 * 
 * 将StmtType枚举值转换为对应的字符串名称，用于调试和日志输出。
 * 
 * @param type 语句类型枚举值
 * @return const char* 语句类型的字符串表示
 */
inline const char *stmt_type_name(StmtType type)
{
  switch (type) {
#define DEFINE_ENUM_ITEM(name) \
  case StmtType::name: return #name;  ///< 返回枚举项的字符串名称
    DEFINE_ENUM()
#undef DEFINE_ENUM_ITEM
    default: return "unkown";  ///< 默认返回未知类型
  }
}

/**
 * @brief 判断语句类型是否为DDL语句
 * 
 * DDL语句包括创建表、删除表、创建索引、删除索引等操作。
 * 
 * @param type 语句类型枚举值
 * @return bool 如果是DDL语句返回true，否则返回false
 */
bool stmt_type_ddl(StmtType type);

/**
 * @brief SQL语句的抽象基类
 * @ingroup Statement
 * @details SQL解析后的语句通过Resolver模块转换成Stmt对象，使用内部的数据结构来表示。
 * 例如，将表名解析成具体的Table对象，将属性名解析成Field对象等。
 * 
 * 所有具体的SQL语句类都必须继承自这个基类，并实现type()方法来返回具体的语句类型。
 */
class Stmt
{
public:
  /**
   * @brief 默认构造函数
   */
  Stmt()          = default;
  
  /**
   * @brief 虚析构函数
   */
  virtual ~Stmt() = default;

  /**
   * @brief 获取语句类型
   * 
   * 纯虚函数，由子类实现，返回具体的语句类型。
   * 
   * @return StmtType 语句类型
   */
  virtual StmtType type() const = 0;

public:
  /**
   * @brief 创建Stmt对象的工厂方法
   * 
   * 根据解析后的SQL节点创建对应的Stmt对象。
   * 
   * @param db 数据库对象
   * @param sql_node 解析后的SQL节点
   * @param stmt 输出参数，创建的Stmt对象
   * @return RC 返回状态码，SUCCESS表示创建成功，其他值表示创建失败
   */
  static RC create_stmt(Db *db, ParsedSqlNode &sql_node, Stmt *&stmt);

private:
  // 禁止拷贝构造和赋值操作
  Stmt(const Stmt &) = delete;
  Stmt &operator=(const Stmt &) = delete;
};
