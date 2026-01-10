/**
 * @file select_stmt.h
 * @brief SELECT语句的核心头文件
 * 
 * 该文件定义了SelectStmt类，用于表示SELECT语句的抽象语法树。
 * SelectStmt类继承自Stmt基类，包含了SELECT语句的所有信息，如查询的表、
 * 查询的表达式、过滤条件和分组信息等。
 * 
 * @author Wangyunlai
 * @date 2022年6月5日
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

#include "common/sys/rc.h"             // 状态码定义
#include "sql/stmt/stmt.h"            // 语句基类
#include "storage/field/field.h"      // 字段定义

class FieldMeta;   // 前向声明字段元数据类
class FilterStmt;  // 前向声明过滤语句类
class Db;          // 前向声明数据库类
class Table;       // 前向声明表类

/**
 * @defgroup Statement 语句模块
 * @brief SQL语句的抽象表示
 * 
 * 语句模块定义了各种SQL语句的抽象表示，包括SELECT、INSERT、DELETE、UPDATE等。
 * 这些类是解析器和优化器之间的桥梁，存储了SQL语句的所有信息。
 */

/**
 * @brief SELECT语句类
 * @ingroup Statement
 * 
 * SelectStmt用于表示SELECT语句的抽象语法树，包含了SELECT语句的所有信息：
 * - 查询的表达式（SELECT子句）
 * - 查询的表（FROM子句）
 * - 过滤条件（WHERE子句）
 * - 分组信息（GROUP BY子句）
 */
class SelectStmt : public Stmt
{
public:
  /**
   * @brief 默认构造函数
   */
  SelectStmt() = default;

  /**
   * @brief 析构函数
   */
  ~SelectStmt() override;

  /**
   * @brief 获取语句类型
   * 
   * @return StmtType 语句类型，这里返回StmtType::SELECT
   */
  StmtType type() const override { return StmtType::SELECT; }

public:
  /**
   * @brief 创建SelectStmt对象
   * 
   * 该静态方法用于从解析后的SQL节点创建SelectStmt对象。
   * 
   * @param db 数据库对象
   * @param select_sql 解析后的SELECT SQL节点
   * @param stmt 输出参数，创建的SelectStmt对象
   * @return RC 返回状态码，SUCCESS表示创建成功，其他值表示创建失败
   */
  static RC create(Db *db, SelectSqlNode &select_sql, Stmt *&stmt);

public:
  /**
   * @brief 获取查询的表列表
   * 
   * @return const vector<Table *>& 查询的表列表
   */
  const vector<Table *> &tables() const { return tables_; }

  /**
   * @brief 获取过滤条件语句
   * 
   * @return FilterStmt* 过滤条件语句，可能为nullptr
   */
  FilterStmt *filter_stmt() const { return filter_stmt_; }

  /**
   * @brief 获取查询表达式列表
   * 
   * 查询表达式列表对应SELECT子句中的表达式，如列名、常量、函数调用等。
   * 
   * @return vector<unique_ptr<Expression>>& 查询表达式列表
   */
  vector<unique_ptr<Expression>> &query_expressions() { return query_expressions_; }

  /**
   * @brief 获取分组表达式列表
   * 
   * 分组表达式列表对应GROUP BY子句中的表达式。
   * 
   * @return vector<unique_ptr<Expression>>& 分组表达式列表
   */
  vector<unique_ptr<Expression>> &group_by() { return group_by_; }

private:
  vector<unique_ptr<Expression>> query_expressions_;  ///< 查询表达式列表（SELECT子句）
  vector<Table *>                tables_;              ///< 查询的表列表（FROM子句）
  FilterStmt                    *filter_stmt_ = nullptr; ///< 过滤条件语句（WHERE子句）
  vector<unique_ptr<Expression>> group_by_;            ///< 分组表达式列表（GROUP BY子句）
};
