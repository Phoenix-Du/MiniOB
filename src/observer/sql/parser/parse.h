/**
 * @file parse.h
 * @brief SQL解析器的核心接口
 * 
 * 该文件定义了SQL解析器的入口函数，用于将SQL字符串解析为内部数据结构。
 * 解析过程包括词法分析和语法分析两个阶段，最终生成SQL语句的抽象表示。
 * 
 * @author Meiyi
 * @date 2021年
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

/**
 * @brief SQL解析的入口函数
 * 
 * 将SQL字符串解析为内部数据结构，用于后续的执行。
 * 
 * @param st 要解析的SQL字符串
 * @param sql_result 解析结果的存储位置
 * @return RC 返回状态码，SUCCESS表示解析成功，其他值表示解析失败
 */
RC parse(const char *st, ParsedSqlResult *sql_result);
