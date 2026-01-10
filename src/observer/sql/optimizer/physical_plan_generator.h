/**
 * @file physical_plan_generator.h
 * @brief 物理计划生成器的核心头文件
 * 
 * 该文件定义了PhysicalPlanGenerator类，负责将逻辑执行计划转换为物理执行计划。
 * 物理执行计划描述了如何执行SQL语句，比如使用哪种扫描方式、连接算法等。
 * 
 * @author Wangyunlai
 * @date 2022年12月14日
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

#include "common/sys/rc.h"                 // 状态码定义
#include "sql/operator/logical_operator.h"   // 逻辑操作符定义
#include "sql/operator/physical_operator.h"  // 物理操作符定义

class Session;                    // 前向声明会话类
class TableGetLogicalOperator;    // 前向声明表获取逻辑操作符类
class PredicateLogicalOperator;   // 前向声明谓词逻辑操作符类
class ProjectLogicalOperator;     // 前向声明投影逻辑操作符类
class InsertLogicalOperator;      // 前向声明插入逻辑操作符类
class DeleteLogicalOperator;      // 前向声明删除逻辑操作符类
class UpdateLogicalOperator;      // 前向声明更新逻辑操作符类
class ExplainLogicalOperator;     // 前向声明解释逻辑操作符类
class JoinLogicalOperator;        // 前向声明连接逻辑操作符类
class CalcLogicalOperator;        // 前向声明计算逻辑操作符类
class GroupByLogicalOperator;     // 前向声明分组逻辑操作符类

/**
 * @defgroup Optimizer 优化器模块
 * @brief SQL语句的优化和执行计划生成
 */

/**
 * @brief 物理计划生成器类
 * @ingroup Optimizer
 * 
 * PhysicalPlanGenerator负责将逻辑执行计划转换为物理执行计划。
 * 物理执行计划描述了如何执行SQL语句，比如使用哪种扫描方式、连接算法等。
 * 
 * 该生成器不会做任何优化，而是完全根据逻辑计划的本意生成物理计划。
 * 它支持生成两种类型的物理计划：
 * 1. 传统的行式执行计划
 * 2. 向量式执行计划（Vectorized Execution Plan）
 */
class PhysicalPlanGenerator
{
public:
  /**
   * @brief 默认构造函数
   */
  PhysicalPlanGenerator()          = default;

  /**
   * @brief 虚析构函数
   */
  virtual ~PhysicalPlanGenerator() = default;

  /**
   * @brief 为逻辑执行计划创建传统的行式物理执行计划
   * 
   * @param logical_operator 逻辑执行计划
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create(LogicalOperator &logical_operator, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为逻辑执行计划创建向量式物理执行计划
   * 
   * 向量式执行计划支持批量处理数据，提高查询性能。
   * 
   * @param logical_operator 逻辑执行计划
   * @param oper 输出参数，生成的向量式物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_vec(LogicalOperator &logical_operator, unique_ptr<PhysicalOperator> &oper, Session *session);

private:
  /**
   * @brief 为表获取逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 表获取逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(TableGetLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为谓词逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 谓词逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(PredicateLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为投影逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 投影逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(ProjectLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为插入逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 插入逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(InsertLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为删除逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 删除逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(DeleteLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为更新逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 更新逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(UpdateLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为解释逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 解释逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(ExplainLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为连接逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 连接逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(JoinLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为计算逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 计算逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(CalcLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为分组逻辑操作符创建物理执行计划
   * 
   * @param logical_oper 分组逻辑操作符
   * @param oper 输出参数，生成的物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_plan(GroupByLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为投影逻辑操作符创建向量式物理执行计划
   * 
   * @param logical_oper 投影逻辑操作符
   * @param oper 输出参数，生成的向量式物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_vec_plan(ProjectLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为表获取逻辑操作符创建向量式物理执行计划
   * 
   * @param logical_oper 表获取逻辑操作符
   * @param oper 输出参数，生成的向量式物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_vec_plan(TableGetLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为分组逻辑操作符创建向量式物理执行计划
   * 
   * @param logical_oper 分组逻辑操作符
   * @param oper 输出参数，生成的向量式物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_vec_plan(GroupByLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 为解释逻辑操作符创建向量式物理执行计划
   * 
   * @param logical_oper 解释逻辑操作符
   * @param oper 输出参数，生成的向量式物理执行计划
   * @param session 会话对象
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC create_vec_plan(ExplainLogicalOperator &logical_oper, unique_ptr<PhysicalOperator> &oper, Session *session);

  /**
   * @brief 判断是否可以使用哈希连接算法
   * 
   * @param logical_oper 连接逻辑操作符
   * @return bool 是否可以使用哈希连接算法
   * 
   * @todo 移除该方法，使用基于代价的优化器（CBO）规则来选择连接算法
   */
  bool can_use_hash_join(JoinLogicalOperator &logical_oper);
};
