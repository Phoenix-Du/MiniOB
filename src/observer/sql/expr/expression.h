/**
 * @file expression.h
 * @brief SQL表达式的核心头文件
 * 
 * 该文件定义了SQL表达式的核心数据结构和接口，包括各种类型的表达式类
 * 和表达式的操作方法。表达式是SQL查询中的核心元素，用于表示各种计算、
 * 比较和转换操作。
 * 
 * @author Wangyunlai
 * @date 2022年7月5日
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

#include "common/lang/string.h"         // 字符串处理
#include "common/lang/memory.h"         // 内存管理
#include "common/lang/unordered_set.h"  // 无序集合
#include "common/value.h"               // 值类型定义
#include "storage/field/field.h"        // 字段定义
#include "sql/expr/aggregator.h"        // 聚合函数定义
#include "storage/common/chunk.h"       // 数据块定义

class Tuple;  // 前向声明元组类

/**
 * @defgroup Expression 表达式模块
 * @brief SQL表达式的定义和操作
 * 
 * 这个模块定义了SQL表达式的核心数据结构和接口，包括各种类型的表达式类
 * 和表达式的操作方法。表达式是SQL查询中的核心元素，用于表示各种计算、
 * 比较和转换操作。
 */

/**
 * @defgroup Expression
 * @brief 表达式
 */

/**
 * @brief 表达式类型枚举
 * @ingroup Expression
 * 
 * 定义了系统支持的所有表达式类型，包括未绑定表达式（需要在resolver阶段解析）
 * 和已绑定表达式（可以直接执行）。
 */
enum class ExprType
{
  NONE,                     ///< 无效表达式类型
  STAR,                     ///< 星号，表示所有字段（如SELECT * FROM table）
  UNBOUND_FIELD,            ///< 未绑定的字段，需要在resolver阶段解析为FieldExpr
  UNBOUND_AGGREGATION,      ///< 未绑定的聚合函数，需要在resolver阶段解析为AggregateExpr

  FIELD,                    ///< 字段表达式。在实际执行时，根据行数据内容提取对应字段的值
  VALUE,                    ///< 常量值表达式
  CAST,                     ///< 需要做类型转换的表达式
  COMPARISON,               ///< 比较表达式（如a > b）
  CONJUNCTION,              ///< 多个表达式使用同一种关系(AND或OR)来联结
  ARITHMETIC,               ///< 算术运算表达式（如a + b）
  AGGREGATION,              ///< 聚合运算表达式（如SUM(a)）
};

/**
 * @brief 表达式的抽象基类
 * @ingroup Expression
 * @details 在SQL的元素中，任何需要得出值的元素都可以使用表达式来描述，
 * 比如获取某个字段的值、比较运算、类型转换、算术运算等。
 * 
 * 通常表达式的值是在真实的算子运算过程中，拿到具体的tuple后才能计算出来。
 * 但是有些表达式可能就表示某一个固定的值，比如ValueExpr。
 * 
 * TODO 区分unbound和bound的表达式
 */
class Expression
{
public:
  /**
   * @brief 默认构造函数
   */
  Expression() = default;

  /**
   * @brief 虚析构函数
   */
  virtual ~Expression() = default;

  /**
   * @brief 复制表达式
   * 
   * 创建并返回当前表达式的深拷贝
   * 
   * @return unique_ptr<Expression> 当前表达式的深拷贝
   */
  virtual unique_ptr<Expression> copy() const = 0;

  /**
   * @brief 判断两个表达式是否相等
   * 
   * 默认实现返回false，子类可以根据需要重写
   * 
   * @param other 要比较的另一个表达式
   * @return bool 如果两个表达式相等则返回true，否则返回false
   */
  virtual bool equal(const Expression &other) const { return false; }
  
  /**
   * @brief 根据具体的tuple，计算当前表达式的值
   * 
   * tuple有可能是一个具体某个表的行数据
   * 
   * @param tuple 用于计算表达式值的元组
   * @param value 输出参数，存储表达式的计算结果
   * @return RC 返回状态码，SUCCESS表示计算成功，其他值表示计算失败
   */
  virtual RC get_value(const Tuple &tuple, Value &value) const = 0;

  /**
   * @brief 在没有实际运行的情况下，尝试获取表达式的值
   * 
   * 有些表达式的值是固定的，比如ValueExpr，这种情况下可以直接获取值
   * 
   * @param value 输出参数，存储表达式的计算结果
   * @return RC 返回状态码，SUCCESS表示获取成功，UNIMPLEMENTED表示当前表达式类型不支持该操作
   */
  virtual RC try_get_value(Value &value) const { return RC::UNIMPLEMENTED; }

  /**
   * @brief 从chunk中获取表达式的计算结果column
   * 
   * 批量计算表达式的值，将结果存储在column中
   * 
   * @param chunk 用于计算表达式值的数据块
   * @param column 输出参数，存储表达式的批量计算结果
   * @return RC 返回状态码，SUCCESS表示计算成功，UNIMPLEMENTED表示当前表达式类型不支持该操作
   */
  virtual RC get_column(Chunk &chunk, Column &column) { return RC::UNIMPLEMENTED; }

  /**
   * @brief 获取表达式的类型
   * 
   * 可以根据表达式类型来转换为具体的子类
   * 
   * @return ExprType 表达式的类型
   */
  virtual ExprType type() const = 0;

  /**
   * @brief 获取表达式值的类型
   * 
   * 一个表达式运算出结果后，只有一个值，该方法返回这个值的类型
   * 
   * @return AttrType 表达式值的类型
   */
  virtual AttrType value_type() const = 0;

  /**
   * @brief 获取表达式值的长度
   * 
   * 对于字符串类型的表达式，返回字符串的最大长度；对于数值类型，返回类型的大小
   * 
   * @return int 表达式值的长度，-1表示长度可变或未知
   */
  virtual int value_length() const { return -1; }

  /**
   * @brief 获取表达式的名字
   * 
   * 比如是字段名称，或者用户在执行SQL语句时输入的内容
   * 
   * @return const char* 表达式的名字
   */
  virtual const char *name() const { return name_.c_str(); }
  
  /**
   * @brief 设置表达式的名字
   * 
   * @param name 表达式的新名字
   */
  virtual void set_name(string name) { name_ = name; }

  /**
   * @brief 获取表达式在下层算子返回的chunk中的位置
   * 
   * 当pos_ >= 0时，表示在下层算子中已经计算出该表达式的值，且该表达式对应的结果位于
   * chunk中下标为pos_的列中；当pos_ = -1时，表示下层算子没有在返回的chunk中计算出该表达式的计算结果
   * 
   * @return int 表达式在chunk中的位置
   */
  virtual int pos() const { return pos_; }
  
  /**
   * @brief 设置表达式在下层算子返回的chunk中的位置
   * 
   * @param pos 表达式在chunk中的新位置
   */
  virtual void set_pos(int pos) { pos_ = pos; }

  /**
   * @brief 用于ComparisonExpr获得比较结果select
   * 
   * 批量计算比较表达式的值，将结果存储在select中
   * 
   * @param chunk 用于计算表达式值的数据块
   * @param select 输出参数，存储比较结果，每个元素表示对应行是否满足条件
   * @return RC 返回状态码，SUCCESS表示计算成功，UNIMPLEMENTED表示当前表达式类型不支持该操作
   */
  virtual RC eval(Chunk &chunk, vector<uint8_t> &select) { return RC::UNIMPLEMENTED; }

protected:
  int pos_ = -1;  ///< 表达式在下层算子返回的chunk中的位置

private:
  string name_;   ///< 表达式的名字
};

/**
 * @brief 星号表达式
 * @ingroup Expression
 * @details 表示所有字段（如SELECT * FROM table），可以指定表名（如SELECT table.* FROM table）
 */
class StarExpr : public Expression
{
public:
  /**
   * @brief 默认构造函数
   */
  StarExpr() : table_name_() {}
  
  /**
   * @brief 构造函数
   * 
   * @param table_name 表名，如果为nullptr表示所有表的所有字段
   */
  StarExpr(const char *table_name) : table_name_(table_name) {}
  
  /**
   * @brief 虚析构函数
   */
  virtual ~StarExpr() = default;

  /**
   * @brief 复制表达式
   * 
   * @return unique_ptr<Expression> 当前表达式的深拷贝
   */
  unique_ptr<Expression> copy() const override { return make_unique<StarExpr>(table_name_.c_str()); }

  /**
   * @brief 获取表达式类型
   * 
   * @return ExprType 表达式类型，返回ExprType::STAR
   */
  ExprType type() const override { return ExprType::STAR; }
  
  /**
   * @brief 获取表达式值的类型
   * 
   * 星号表达式不表示具体的值，因此返回UNDEFINED
   * 
   * @return AttrType 表达式值的类型，返回AttrType::UNDEFINED
   */
  AttrType value_type() const override { return AttrType::UNDEFINED; }

  /**
   * @brief 根据tuple计算表达式的值
   * 
   * 星号表达式不表示具体的值，因此该方法未实现
   * 
   * @return RC 返回RC::UNIMPLEMENTED
   */
  RC get_value(const Tuple &tuple, Value &value) const override { return RC::UNIMPLEMENTED; }  // 不需要实现

  /**
   * @brief 获取表名
   * 
   * @return const char* 表名，如果为空表示所有表
   */
  const char *table_name() const { return table_name_.c_str(); }

private:
  string table_name_;  ///< 表名，如果为空表示所有表
};

/**
 * @brief 未绑定的字段表达式
 * @ingroup Expression
 * @details 需要在resolver阶段解析为FieldExpr
 */
class UnboundFieldExpr : public Expression
{
public:
  /**
   * @brief 构造函数
   * 
   * @param table_name 表名
   * @param field_name 字段名
   */
  UnboundFieldExpr(const string &table_name, const string &field_name)
      : table_name_(table_name), field_name_(field_name)
  {}

  /**
   * @brief 虚析构函数
   */
  virtual ~UnboundFieldExpr() = default;

  /**
   * @brief 复制表达式
   * 
   * @return unique_ptr<Expression> 当前表达式的深拷贝
   */
  unique_ptr<Expression> copy() const override { return make_unique<UnboundFieldExpr>(table_name_, field_name_); }

  /**
   * @brief 获取表达式类型
   * 
   * @return ExprType 表达式类型，返回ExprType::UNBOUND_FIELD
   */
  ExprType type() const override { return ExprType::UNBOUND_FIELD; }
  
  /**
   * @brief 获取表达式值的类型
   * 
   * 未绑定的字段表达式还未解析，因此返回UNDEFINED
   * 
   * @return AttrType 表达式值的类型，返回AttrType::UNDEFINED
   */
  AttrType value_type() const override { return AttrType::UNDEFINED; }

  /**
   * @brief 根据tuple计算表达式的值
   * 
   * 未绑定的字段表达式还未解析，因此该方法返回内部错误
   * 
   * @return RC 返回RC::INTERNAL
   */
  RC get_value(const Tuple &tuple, Value &value) const override { return RC::INTERNAL; }

  /**
   * @brief 获取表名
   * 
   * @return const char* 表名
   */
  const char *table_name() const { return table_name_.c_str(); }
  
  /**
   * @brief 获取字段名
   * 
   * @return const char* 字段名
   */
  const char *field_name() const { return field_name_.c_str(); }

private:
  string table_name_;  ///< 表名
  string field_name_;  ///< 字段名
};

/**
 * @brief 字段表达式
 * @ingroup Expression
 * @details 表示表中的一个字段，在实际执行时，根据行数据内容提取对应字段的值
 */
class FieldExpr : public Expression
{
public:
  /**
   * @brief 默认构造函数
   */
  FieldExpr() = default;
  
  /**
   * @brief 构造函数
   * 
   * @param table 表对象
   * @param field 字段元数据
   */
  FieldExpr(const Table *table, const FieldMeta *field) : field_(table, field) {}
  
  /**
   * @brief 构造函数
   * 
   * @param field 字段对象
   */
  FieldExpr(const Field &field) : field_(field) {}

  /**
   * @brief 虚析构函数
   */
  virtual ~FieldExpr() = default;

  /**
   * @brief 判断两个表达式是否相等
   * 
   * @param other 要比较的另一个表达式
   * @return bool 如果两个表达式相等则返回true，否则返回false
   */
  bool equal(const Expression &other) const override;

  /**
   * @brief 复制表达式
   * 
   * @return unique_ptr<Expression> 当前表达式的深拷贝
   */
  unique_ptr<Expression> copy() const override { return make_unique<FieldExpr>(field_); }

  /**
   * @brief 获取表达式类型
   * 
   * @return ExprType 表达式类型，返回ExprType::FIELD
   */
  ExprType type() const override { return ExprType::FIELD; }
  
  /**
   * @brief 获取表达式值的类型
   * 
   * @return AttrType 表达式值的类型，即字段的类型
   */
  AttrType value_type() const override { return field_.attr_type(); }
  
  /**
   * @brief 获取表达式值的长度
   * 
   * @return int 表达式值的长度，即字段的长度
   */
  int      value_length() const override { return field_.meta()->len(); }

  /**
   * @brief 获取字段对象（非const版本）
   * 
   * @return Field& 字段对象
   */
  Field &field() { return field_; }

  /**
   * @brief 获取字段对象（const版本）
   * 
   * @return const Field& 字段对象
   */
  const Field &field() const { return field_; }

  /**
   * @brief 获取表名
   * 
   * @return const char* 表名
   */
  const char *table_name() const { return field_.table_name(); }
  
  /**
   * @brief 获取字段名
   * 
   * @return const char* 字段名
   */
  const char *field_name() const { return field_.field_name(); }

  /**
   * @brief 从chunk中获取字段的计算结果column
   * 
   * 批量获取字段的值，将结果存储在column中
   * 
   * @param chunk 用于获取字段值的数据块
   * @param column 输出参数，存储字段的批量值
   * @return RC 返回状态码，SUCCESS表示获取成功
   */
  RC get_column(Chunk &chunk, Column &column) override;

  /**
   * @brief 根据tuple计算字段的值
   * 
   * @param tuple 用于获取字段值的元组
   * @param value 输出参数，存储字段的值
   * @return RC 返回状态码，SUCCESS表示获取成功
   */
  RC get_value(const Tuple &tuple, Value &value) const override;

private:
  Field field_;  ///< 字段对象
};

/**
 * @brief 常量值表达式
 * @ingroup Expression
 */
class ValueExpr : public Expression
{
public:
  ValueExpr() = default;
  explicit ValueExpr(const Value &value) : value_(value) {}

  virtual ~ValueExpr() = default;

  bool equal(const Expression &other) const override;

  unique_ptr<Expression> copy() const override { return make_unique<ValueExpr>(value_); }

  RC get_value(const Tuple &tuple, Value &value) const override;
  RC get_column(Chunk &chunk, Column &column) override;
  RC try_get_value(Value &value) const override
  {
    value = value_;
    return RC::SUCCESS;
  }

  ExprType type() const override { return ExprType::VALUE; }
  AttrType value_type() const override { return value_.attr_type(); }
  int      value_length() const override { return value_.length(); }

  void         get_value(Value &value) const { value = value_; }
  const Value &get_value() const { return value_; }

private:
  Value value_;
};

/**
 * @brief 类型转换表达式
 * @ingroup Expression
 */
class CastExpr : public Expression
{
public:
  CastExpr(unique_ptr<Expression> child, AttrType cast_type);
  virtual ~CastExpr();

  unique_ptr<Expression> copy() const override { return make_unique<CastExpr>(child_->copy(), cast_type_); }

  ExprType type() const override { return ExprType::CAST; }

  RC get_value(const Tuple &tuple, Value &value) const override;
  RC get_column(Chunk &chunk, Column &column) override;

  RC try_get_value(Value &value) const override;

  AttrType value_type() const override { return cast_type_; }

  unique_ptr<Expression> &child() { return child_; }

private:
  RC cast(const Value &value, Value &cast_value) const;

private:
  unique_ptr<Expression> child_;      ///< 从这个表达式转换
  AttrType               cast_type_;  ///< 想要转换成这个类型
};

/**
 * @brief 比较表达式
 * @ingroup Expression
 */
class ComparisonExpr : public Expression
{
public:
  ComparisonExpr(CompOp comp, unique_ptr<Expression> left, unique_ptr<Expression> right);
  virtual ~ComparisonExpr();

  ExprType type() const override { return ExprType::COMPARISON; }
  RC       get_value(const Tuple &tuple, Value &value) const override;
  AttrType value_type() const override { return AttrType::BOOLEANS; }
  CompOp   comp() const { return comp_; }

  unique_ptr<Expression> copy() const override
  {
    return make_unique<ComparisonExpr>(comp_, left_->copy(), right_->copy());
  }

  /**
   * @brief 根据 ComparisonExpr 获得 `select` 结果。
   * select 的长度与chunk 的行数相同，表示每一行在ComparisonExpr 计算后是否会被输出。
   */
  RC eval(Chunk &chunk, vector<uint8_t> &select) override;

  unique_ptr<Expression> &left() { return left_; }
  unique_ptr<Expression> &right() { return right_; }

  /**
   * 尝试在没有tuple的情况下获取当前表达式的值
   * 在优化的时候，可能会使用到
   */
  RC try_get_value(Value &value) const override;

  /**
   * compare the two tuple cells
   * @param value the result of comparison
   */
  RC compare_value(const Value &left, const Value &right, bool &value) const;

  template <typename T>
  RC compare_column(const Column &left, const Column &right, vector<uint8_t> &result) const;

private:
  CompOp                 comp_;
  unique_ptr<Expression> left_;
  unique_ptr<Expression> right_;
};

/**
 * @brief 联结表达式
 * @ingroup Expression
 * 多个表达式使用同一种关系(AND或OR)来联结
 * 当前miniob仅有AND操作
 */
class ConjunctionExpr : public Expression
{
public:
  enum class Type
  {
    AND,
    OR
  };

public:
  ConjunctionExpr(Type type, vector<unique_ptr<Expression>> &children);
  virtual ~ConjunctionExpr() = default;

  unique_ptr<Expression> copy() const override
  {
    vector<unique_ptr<Expression>> children;
    for (auto &child : children_) {
      children.emplace_back(child->copy());
    }
    return make_unique<ConjunctionExpr>(conjunction_type_, children);
  }

  ExprType type() const override { return ExprType::CONJUNCTION; }
  AttrType value_type() const override { return AttrType::BOOLEANS; }
  RC       get_value(const Tuple &tuple, Value &value) const override;

  Type conjunction_type() const { return conjunction_type_; }

  vector<unique_ptr<Expression>> &children() { return children_; }

private:
  Type                           conjunction_type_;
  vector<unique_ptr<Expression>> children_;
};

/**
 * @brief 算术表达式
 * @ingroup Expression
 */
class ArithmeticExpr : public Expression
{
public:
  enum class Type
  {
    ADD,
    SUB,
    MUL,
    DIV,
    NEGATIVE,
  };

public:
  ArithmeticExpr(Type type, Expression *left, Expression *right);
  ArithmeticExpr(Type type, unique_ptr<Expression> left, unique_ptr<Expression> right);
  virtual ~ArithmeticExpr() = default;

  unique_ptr<Expression> copy() const override
  {
    if (right_) {
      return make_unique<ArithmeticExpr>(arithmetic_type_, left_->copy(), right_->copy());
    } else {
      return make_unique<ArithmeticExpr>(arithmetic_type_, left_->copy(), nullptr);
    }
  }

  bool     equal(const Expression &other) const override;
  ExprType type() const override { return ExprType::ARITHMETIC; }

  AttrType value_type() const override;
  int value_length() const override { return std::max(left_->value_length(), right_ ? right_->value_length() : 0); };

  RC get_value(const Tuple &tuple, Value &value) const override;

  RC get_column(Chunk &chunk, Column &column) override;

  RC try_get_value(Value &value) const override;

  Type arithmetic_type() const { return arithmetic_type_; }

  unique_ptr<Expression> &left() { return left_; }
  unique_ptr<Expression> &right() { return right_; }

private:
  RC calc_value(const Value &left_value, const Value &right_value, Value &value) const;

  RC calc_column(const Column &left_column, const Column &right_column, Column &column) const;

  template <bool LEFT_CONSTANT, bool RIGHT_CONSTANT>
  RC execute_calc(const Column &left, const Column &right, Column &result, Type type, AttrType attr_type) const;

private:
  Type                   arithmetic_type_;
  unique_ptr<Expression> left_;
  unique_ptr<Expression> right_;
};

class UnboundAggregateExpr : public Expression
{
public:
  UnboundAggregateExpr(const char *aggregate_name, Expression *child);
  UnboundAggregateExpr(const char *aggregate_name, unique_ptr<Expression> child);
  virtual ~UnboundAggregateExpr() = default;

  ExprType type() const override { return ExprType::UNBOUND_AGGREGATION; }

  unique_ptr<Expression> copy() const override
  {
    return make_unique<UnboundAggregateExpr>(aggregate_name_.c_str(), child_->copy());
  }

  const char *aggregate_name() const { return aggregate_name_.c_str(); }

  unique_ptr<Expression> &child() { return child_; }

  RC       get_value(const Tuple &tuple, Value &value) const override { return RC::INTERNAL; }
  AttrType value_type() const override { return child_->value_type(); }

private:
  string                 aggregate_name_;
  unique_ptr<Expression> child_;
};

class AggregateExpr : public Expression
{
public:
  enum class Type
  {
    COUNT,
    SUM,
    AVG,
    MAX,
    MIN,
  };

public:
  AggregateExpr(Type type, Expression *child);
  AggregateExpr(Type type, unique_ptr<Expression> child);
  virtual ~AggregateExpr() = default;

  bool equal(const Expression &other) const override;

  unique_ptr<Expression> copy() const override { return make_unique<AggregateExpr>(aggregate_type_, child_->copy()); }

  ExprType type() const override { return ExprType::AGGREGATION; }

  AttrType value_type() const override
  {
    if (aggregate_type_ == Type::COUNT) {
      return AttrType::INTS;
    } else if (aggregate_type_ == Type::AVG) {
      return AttrType::FLOATS;
    } else {
      return child_->value_type();
    }
  }
  int value_length() const override
  {
    if (aggregate_type_ == Type::COUNT) {
      return sizeof(int);
    } else if (aggregate_type_ == Type::AVG) {
      return sizeof(float);
    } else {
      return child_->value_length();
    }
  }

  RC get_value(const Tuple &tuple, Value &value) const override;

  RC get_column(Chunk &chunk, Column &column) override;

  Type aggregate_type() const { return aggregate_type_; }

  unique_ptr<Expression> &child() { return child_; }

  const unique_ptr<Expression> &child() const { return child_; }

  unique_ptr<Aggregator> create_aggregator() const;

public:
  static RC type_from_string(const char *type_str, Type &type);

private:
  Type                   aggregate_type_;
  unique_ptr<Expression> child_;
};
