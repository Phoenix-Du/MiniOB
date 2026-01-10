/**
 * @file filter_stmt.h
 * @brief 过滤语句（谓词）的核心头文件
 * 
 * 该文件定义了过滤语句的相关结构，包括FilterObj（过滤对象）、FilterUnit（过滤单元）
 * 和FilterStmt（过滤语句）。这些结构用于表示SQL语句中的WHERE子句和HAVING子句的过滤条件。
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

#include "common/lang/unordered_map.h"   // 无序映射
#include "common/lang/vector.h"          // 向量容器
#include "sql/expr/expression.h"         // 表达式定义
#include "sql/parser/parse_defs.h"       // SQL解析相关定义
#include "sql/stmt/stmt.h"               // 语句基类

class Db;          // 前向声明数据库类
class Table;       // 前向声明表类
class FieldMeta;   // 前向声明字段元数据类

/**
 * @defgroup Statement 语句模块
 * @brief SQL语句的抽象表示
 */

/**
 * @brief 过滤对象结构
 * @ingroup Statement
 * 
 * FilterObj表示过滤条件中的一个操作数，可以是字段（属性）或值。
 */
struct FilterObj
{
  bool  is_attr;    ///< 是否为字段（属性），true表示是字段，false表示是值
  Field field;      ///< 字段对象，当is_attr为true时有效
  Value value;      ///< 值对象，当is_attr为false时有效

  /**
   * @brief 初始化字段对象
   * 
   * @param field 字段对象
   */
  void init_attr(const Field &field)
  {
    is_attr     = true;
    this->field = field;
  }

  /**
   * @brief 初始化值对象
   * 
   * @param value 值对象
   */
  void init_value(const Value &value)
  {
    is_attr     = false;
    this->value = value;
  }
};

/**
 * @brief 过滤单元类
 * @ingroup Statement
 * 
 * FilterUnit表示一个完整的过滤条件单元，如"column > value"或"column1 = column2"。
 * 它包含左操作数、比较运算符和右操作数。
 */
class FilterUnit
{
public:
  /**
   * @brief 默认构造函数
   */
  FilterUnit() = default;

  /**
   * @brief 析构函数
   */
  ~FilterUnit() {}

  /**
   * @brief 设置比较运算符
   * 
   * @param comp 比较运算符
   */
  void set_comp(CompOp comp) { comp_ = comp; }

  /**
   * @brief 获取比较运算符
   * 
   * @return CompOp 比较运算符
   */
  CompOp comp() const { return comp_; }

  /**
   * @brief 设置左操作数
   * 
   * @param obj 左操作数
   */
  void set_left(const FilterObj &obj) { left_ = obj; }

  /**
   * @brief 设置右操作数
   * 
   * @param obj 右操作数
   */
  void set_right(const FilterObj &obj) { right_ = obj; }

  /**
   * @brief 获取左操作数
   * 
   * @return const FilterObj& 左操作数
   */
  const FilterObj &left() const { return left_; }

  /**
   * @brief 获取右操作数
   * 
   * @return const FilterObj& 右操作数
   */
  const FilterObj &right() const { return right_; }

private:
  CompOp    comp_ = NO_OP;  ///< 比较运算符
  FilterObj left_;          ///< 左操作数
  FilterObj right_;         ///< 右操作数
};

/**
 * @brief 过滤语句类
 * @ingroup Statement
 * 
 * FilterStmt表示SQL语句中的WHERE子句或HAVING子句的过滤条件。
 * 它包含多个FilterUnit，默认情况下这些过滤单元之间是AND关系。
 */
class FilterStmt
{
public:
  /**
   * @brief 默认构造函数
   */
  FilterStmt() = default;

  /**
   * @brief 虚析构函数
   */
  virtual ~FilterStmt();

public:
  /**
   * @brief 获取过滤单元列表
   * 
   * @return const vector<FilterUnit *>& 过滤单元列表
   */
  const vector<FilterUnit *> &filter_units() const { return filter_units_; }

public:
  /**
   * @brief 创建FilterStmt对象
   * 
   * 该静态方法用于从解析后的条件SQL节点创建FilterStmt对象。
   * 
   * @param db 数据库对象
   * @param default_table 默认表对象
   * @param tables 表名到表对象的映射
   * @param conditions 条件SQL节点数组
   * @param condition_num 条件数量
   * @param stmt 输出参数，创建的FilterStmt对象
   * @return RC 返回状态码，SUCCESS表示创建成功，其他值表示创建失败
   */
  static RC create(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
      const ConditionSqlNode *conditions, int condition_num, FilterStmt *&stmt);

  /**
   * @brief 创建FilterUnit对象
   * 
   * 该静态方法用于从解析后的条件SQL节点创建FilterUnit对象。
   * 
   * @param db 数据库对象
   * @param default_table 默认表对象
   * @param tables 表名到表对象的映射
   * @param condition 条件SQL节点
   * @param filter_unit 输出参数，创建的FilterUnit对象
   * @return RC 返回状态码，SUCCESS表示创建成功，其他值表示创建失败
   */
  static RC create_filter_unit(Db *db, Table *default_table, unordered_map<string, Table *> *tables,
      const ConditionSqlNode &condition, FilterUnit *&filter_unit);

private:
  vector<FilterUnit *> filter_units_;  ///< 过滤单元列表，默认这些单元之间是AND关系
};
