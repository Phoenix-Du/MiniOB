/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

//
// Created by Wangyunlai on 2022/12/29.
//

#pragma once

#include "sql/optimizer/rewrite_rule.h"
#include "common/lang/memory.h"
#include "common/lang/vector.h"

class LogicalOperator;

/**
 * @brief 查询重写器
 * @file rewriter.h
 * @ingroup Optimizer
 * 
 * 该文件定义了Rewriter类，用于根据一系列规则对逻辑执行计划进行重写。
 * 重写是查询优化的重要阶段，它在生成物理计划之前对逻辑计划进行转换，
 * 以提高查询执行效率。
 * 
 * @todo 将重写器与级联优化器（cascade optimizer）进行重构整合。
 */

/**
 * @brief 逻辑计划重写器类
 * @ingroup Optimizer
 * 
 * Rewriter类负责应用一系列重写规则对逻辑执行计划进行转换。它维护了一组重写规则，
 * 并依次应用这些规则对逻辑计划进行重写。如果应用某个规则导致逻辑计划发生了变化，
 * 通常会继续应用所有规则，直到没有规则能再对逻辑计划进行修改为止。
 * 
 * 当前实现只包含了一些非常简单的重写规则。重写过程包括对逻辑计划本身和计划中包含的
 * 表达式进行转换。
 */
class Rewriter
{
public:
  /**
   * @brief 构造函数
   * 
   * 初始化重写器并加载所有可用的重写规则。
   */
  Rewriter();

  /**
   * @brief 虚析构函数
   */
  virtual ~Rewriter() = default;

  /**
   * @brief 对逻辑执行计划进行重写
   * 
   * 依次应用所有重写规则对逻辑计划进行转换。如果任何规则导致逻辑计划发生了变化，
   * 则change_made参数会被设置为true。
   * 
   * @param oper 要重写的逻辑执行计划
   * @param change_made 输出参数，表示本次重写是否对逻辑计划进行了修改
   * @return RC 返回状态码，SUCCESS表示重写成功，其他值表示重写失败
   */
  RC rewrite(unique_ptr<LogicalOperator> &oper, bool &change_made);

private:
  /**
   * @brief 重写规则列表
   * 
   * 存储所有可用的重写规则。这些规则会在rewrite方法中依次被应用。
   */
  vector<unique_ptr<RewriteRule>> rewrite_rules_;
};
