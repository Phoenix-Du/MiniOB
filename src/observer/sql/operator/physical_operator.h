/**
 * @file physical_operator.h
 * @brief SQL物理操作符的核心头文件
 * 
 * 该文件定义了SQL物理操作符的抽象基类PhysicalOperator，以及物理操作符的类型枚举。
 * 物理操作符描述了执行计划将如何执行，比如从表中获取数据、如何做投影、怎么做连接等。
 * 
 * @author WangYunlai
 * @date 2022年6月7日
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

#include "common/sys/rc.h"                // 状态码定义
#include "sql/expr/tuple.h"               // 元组定义
#include "sql/operator/operator_node.h"   // 操作符节点基类

class Record;      // 前向声明记录类
class TupleCellSpec; // 前向声明元组单元格规范类
class Trx;         // 前向声明事务类

/**
 * @defgroup PhysicalOperator 物理操作符
 * @brief SQL执行计划的物理实现
 * 
 * 物理操作符描述了执行计划将如何执行，比如从表中获取数据、如何做投影、怎么做连接等。
 * 物理操作符是SQL执行的核心组件，负责实际的数据处理和计算。
 */

/**
 * @brief 物理算子
 * @defgroup PhysicalOperator
 * @details 物理算子描述执行计划将如何执行，比如从表中怎么获取数据，如何做投影，怎么做连接等
 */

/**
 * @brief 物理操作符类型枚举
 * @ingroup PhysicalOperator
 * 
 * 该枚举定义了系统支持的所有物理操作符类型，每种类型对应一种具体的物理执行方式。
 */
enum class PhysicalOperatorType
{
  TABLE_SCAN,           ///< 表扫描操作符，顺序扫描表中的所有记录
  TABLE_SCAN_VEC,       ///< 向量表扫描操作符，支持批量处理的表扫描
  INDEX_SCAN,           ///< 索引扫描操作符，通过索引查找记录
  NESTED_LOOP_JOIN,     ///< 嵌套循环连接操作符
  HASH_JOIN,            ///< 哈希连接操作符
  EXPLAIN,              ///< EXPLAIN操作符，用于解释执行计划
  PREDICATE,            ///< 谓词操作符，用于过滤记录
  PREDICATE_VEC,        ///< 向量谓词操作符，支持批量处理的谓词过滤
  PROJECT,              ///< 投影操作符，选择指定的列
  PROJECT_VEC,          ///< 向量投影操作符，支持批量处理的投影
  CALC,                 ///< 计算操作符，用于执行计算表达式
  STRING_LIST,          ///< 字符串列表操作符，用于生成字符串列表结果
  DELETE,               ///< 删除操作符，用于删除记录
  INSERT,               ///< 插入操作符，用于插入记录
  UPDATE,               ///< 更新操作符，用于更新记录
  SCALAR_GROUP_BY,      ///< 标量分组操作符
  HASH_GROUP_BY,        ///< 哈希分组操作符
  GROUP_BY_VEC,         ///< 向量分组操作符，支持批量处理的分组
  AGGREGATE_VEC,        ///< 向量聚合操作符，支持批量处理的聚合
  EXPR_VEC,             ///< 向量表达式操作符，支持批量处理的表达式计算
};

/**
 * @brief 物理操作符基类
 * @ingroup PhysicalOperator
 * 
 * 物理操作符是SQL执行计划的具体实现，描述了执行计划将如何执行。
 * 所有具体的物理操作符都继承自这个基类。
 */
class PhysicalOperator : public OperatorNode
{
public:
  /**
   * @brief 默认构造函数
   */
  PhysicalOperator() = default;

  /**
   * @brief 虚析构函数，确保子类析构函数被正确调用
   */
  virtual ~PhysicalOperator() = default;

  /**
   * @brief 获取操作符名称，用于explain等场景
   * @return 操作符名称
   */
  virtual string name() const;

  /**
   * @brief 获取操作符参数，用于explain等场景
   * @return 操作符参数描述
   */
  virtual string param() const;

  /**
   * @brief 判断是否为物理操作符
   * @return 总是返回true
   */
  bool is_physical() const override { return true; }

  /**
   * @brief 判断是否为逻辑操作符
   * @return 总是返回false
   */
  bool is_logical() const override { return false; }

  /**
   * @brief 获取物理操作符类型
   * @return 物理操作符类型
   */
  virtual PhysicalOperatorType type() const = 0;

  /**
   * @brief 打开操作符，准备执行
   * @param trx 事务对象
   * @return 操作状态码
   */
  virtual RC open(Trx *trx) = 0;

  /**
   * @brief 获取下一条记录
   * @return 操作状态码，RC::SUCCESS表示成功获取一条记录，RC::RECORD_EOF表示没有更多记录
   */
  virtual RC next() { return RC::UNIMPLEMENTED; }

  /**
   * @brief 获取下一批记录（向量处理）
   * @param chunk 存储记录的块
   * @return 操作状态码，RC::SUCCESS表示成功获取一批记录，RC::RECORD_EOF表示没有更多记录
   */
  virtual RC next(Chunk &chunk) { return RC::UNIMPLEMENTED; }

  /**
   * @brief 关闭操作符，释放资源
   * @return 操作状态码
   */
  virtual RC close() = 0;

  /**
   * @brief 获取当前记录
   * @return 当前记录的指针，可能为nullptr
   */
  virtual Tuple *current_tuple() { return nullptr; }

  /**
   * @brief 获取操作符输出的元组模式
   * @param schema 输出的元组模式
   * @return 操作状态码
   */
  virtual RC tuple_schema(TupleSchema &schema) const { return RC::UNIMPLEMENTED; }

  /**
   * @brief 添加子操作符
   * @param oper 子操作符的智能指针
   */
  void add_child(unique_ptr<PhysicalOperator> oper) { children_.emplace_back(std::move(oper)); }

  /**
   * @brief 获取子操作符列表
   * @return 子操作符列表的引用
   */
  vector<unique_ptr<PhysicalOperator>> &children() { return children_; }

protected:
  vector<unique_ptr<PhysicalOperator>> children_;  ///< 子操作符列表
};
