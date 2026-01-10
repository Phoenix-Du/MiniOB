/**
 * @file table.h
 * @brief 表的核心定义和操作接口
 * 
 * 该文件定义了Table类，它是数据库表的核心表示，提供了表的创建、打开、关闭、删除、记录操作等功能。
 * Table类封装了表的元数据、存储引擎和索引等组件，为上层提供了统一的表操作接口。
 * 
 * @author Meiyi & Wangyunlai
 * @date 2021/5/12
 * @version 1.0
 */

/* Copyright (c) 2021 Xie Meiyi(xiemeiyi@hust.edu.cn) and OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "storage/table/table_meta.h"
#include "storage/table/table_engine.h"
#include "storage/common/chunk.h"
#include "storage/record/lob_handler.h"
#include "common/types.h"
#include "common/lang/span.h"
#include "common/lang/functional.h"

struct RID;
class Record;
class DiskBufferPool;
class RecordFileHandler;
class RecordScanner;
class ChunkFileScanner;
class ConditionFilter;
class DefaultConditionFilter;
class Index;
class IndexScanner;
class RecordDeleter;
class Trx;
class Db;

/**
 * @defgroup Table 表模块
 * @brief 负责表的管理和操作，包括表的创建、打开、关闭、删除以及记录的增删改查等功能
 * 
 * 表模块是存储引擎的核心组件之一，它封装了表的元数据、存储引擎和索引等，为上层提供了统一的表操作接口。
 * 表模块支持事务处理、索引管理和记录操作等功能。
 */

/**
 * @brief 表类
 * @ingroup Table
 * 
 * Table类是数据库表的核心表示，提供了表的创建、打开、关闭、删除以及记录的增删改查等功能。
 * 它封装了表的元数据、存储引擎和索引等组件，为上层提供了统一的表操作接口。
 * 
 * @note Table类目前正在向基于TableEngine的设计模式过渡（见TODO注释）。
 */
class Table
{
public:
  /**
   * @brief 默认构造函数
   */
  Table() = default;
  
  /**
   * @brief 析构函数
   */
  ~Table();

  // TODO: use TableEngine replace Table
  friend class TableEngine;
  friend class HeapTableEngine;

  /**
   * @brief 创建一个表
   * 
   * 该方法根据给定的参数创建一个新的表，包括表的元数据和数据文件。
   * 
   * @param db 数据库对象，表所属的数据库
   * @param table_id 表的唯一标识符
   * @param path 元数据保存的文件(完整路径)
   * @param name 表名
   * @param base_dir 表数据存放的路径
   * @param attributes 字段信息列表
   * @param primary_keys 主键字段名列表
   * @param storage_format 存储格式
   * @param storage_engine 存储引擎类型
   * @return RC 返回状态码，SUCCESS表示创建成功，其他值表示创建失败
   */
  RC create(Db *db, int32_t table_id, const char *path, const char *name, const char *base_dir,
      span<const AttrInfoSqlNode> attributes, const vector<string> &primary_keys, StorageFormat storage_format,
      StorageEngine storage_engine);

  /**
   * @brief 打开一个表
   * 
   * 该方法打开一个已存在的表，加载表的元数据和索引信息。
   * 
   * @param db 数据库对象，表所属的数据库
   * @param meta_file 保存表元数据的文件完整路径
   * @param base_dir 表所在的文件夹，表记录数据文件、索引数据文件存放位置
   * @return RC 返回状态码，SUCCESS表示打开成功，其他值表示打开失败
   */
  RC open(Db *db, const char *meta_file, const char *base_dir);

  /**
   * @brief 根据给定的字段生成一个记录/行
   * 
   * 该方法根据用户传入的字段值，按照表的schema信息组装成一个完整的记录。
   * 
   * @param value_num 字段的个数
   * @param values 每个字段的值
   * @param record 生成的记录数据
   * @return RC 返回状态码，SUCCESS表示生成成功，其他值表示生成失败
   */
  RC make_record(int value_num, const Value *values, Record &record);

  /**
   * @brief 在当前的表中插入一条记录
   * 
   * 该方法在表文件和索引中插入关联数据，但不关心事务相关操作。
   * 
   * @param record[in/out] 传入的数据包含具体的数据，插入成功会通过此字段返回RID
   * @return RC 返回状态码，SUCCESS表示插入成功，其他值表示插入失败
   */
  RC insert_record(Record &record);

  /**
   * @brief 插入一个数据块
   * 
   * 该方法将一个数据块插入到表中。
   * 
   * @param chunk 要插入的数据块
   * @return RC 返回状态码，SUCCESS表示插入成功，其他值表示插入失败
   */
  RC insert_chunk(const Chunk &chunk);
  
  /**
   * @brief 删除一条记录
   * 
   * 该方法从表中删除一条记录，但不关心事务相关操作。
   * 
   * @param record 要删除的记录
   * @return RC 返回状态码，SUCCESS表示删除成功，其他值表示删除失败
   */
  RC delete_record(const Record &record);

  /**
   * @brief 在事务中插入一条记录
   * 
   * 该方法在事务的上下文中插入一条记录，确保事务的ACID特性。
   * 
   * @param record[in/out] 传入的数据包含具体的数据，插入成功会通过此字段返回RID
   * @param trx 事务对象
   * @return RC 返回状态码，SUCCESS表示插入成功，其他值表示插入失败
   */
  RC insert_record_with_trx(Record &record, Trx *trx);
  
  /**
   * @brief 在事务中删除一条记录
   * 
   * 该方法在事务的上下文中删除一条记录，确保事务的ACID特性。
   * 
   * @param record 要删除的记录
   * @param trx 事务对象
   * @return RC 返回状态码，SUCCESS表示删除成功，其他值表示删除失败
   */
  RC delete_record_with_trx(const Record &record, Trx *trx);
  
  /**
   * @brief 在事务中更新一条记录
   * 
   * 该方法在事务的上下文中更新一条记录，确保事务的ACID特性。
   * 
   * @param old_record 旧记录
   * @param new_record 新记录
   * @param trx 事务对象
   * @return RC 返回状态码，SUCCESS表示更新成功，其他值表示更新失败
   */
  RC update_record_with_trx(const Record &old_record, const Record &new_record, Trx *trx);
  
  /**
   * @brief 根据记录ID获取一条记录
   * 
   * 该方法根据记录ID获取对应的记录内容。
   * 
   * @param rid 记录ID
   * @param record 记录对象，用于存储获取到的记录内容
   * @return RC 返回状态码，SUCCESS表示获取成功，其他值表示获取失败
   */
  RC get_record(const RID &rid, Record &record);

  /**
   * @brief 创建一个索引
   * 
   * 该方法为表的指定字段创建一个索引。
   * 
   * @param trx 事务对象
   * @param field_meta 字段元数据
   * @param index_name 索引名
   * @return RC 返回状态码，SUCCESS表示创建成功，其他值表示创建失败
   * 
   * @note 该方法需要重构（见TODO注释）。
   */
  // TODO refactor
  RC create_index(Trx *trx, const FieldMeta *field_meta, const char *index_name);

  /**
   * @brief 获取记录扫描器
   * 
   * 该方法获取一个记录扫描器，用于遍历表中的记录。
   * 
   * @param scanner[out] 扫描器对象指针
   * @param trx 事务对象
   * @param mode 读写模式
   * @return RC 返回状态码，SUCCESS表示获取成功，其他值表示获取失败
   */
  RC get_record_scanner(RecordScanner *&scanner, Trx *trx, ReadWriteMode mode);

  /**
   * @brief 获取数据块扫描器
   * 
   * 该方法获取一个数据块扫描器，用于遍历表中的数据块。
   * 
   * @param scanner[out] 扫描器对象
   * @param trx 事务对象
   * @param mode 读写模式
   * @return RC 返回状态码，SUCCESS表示获取成功，其他值表示获取失败
   */
  RC get_chunk_scanner(ChunkFileScanner &scanner, Trx *trx, ReadWriteMode mode);

  /**
   * @brief 可以在页面锁保护的情况下访问记录
   * 
   * 该方法在页面锁的保护下访问记录，提供一个原子性的访问模式。
   * 
   * @param rid 记录ID
   * @param visitor 访问者函数，用于处理记录
   * @return RC 返回状态码，SUCCESS表示访问成功，其他值表示访问失败
   */
  RC visit_record(const RID &rid, function<bool(Record &)> visitor);

public:
  /**
   * @brief 获取表的唯一标识符
   * 
   * @return int32_t 返回表的唯一标识符
   */
  int32_t table_id() const { return table_meta_.table_id(); }
  
  /**
   * @brief 获取表名
   * 
   * @return const char* 返回表名
   */
  const char *name() const;

  /**
   * @brief 获取数据库对象
   * 
   * @return Db* 返回表所属的数据库对象
   */
  Db *db() const { return db_; }

  /**
   * @brief 获取表的元数据
   * 
   * @return const TableMeta& 返回表的元数据
   */
  const TableMeta &table_meta() const;

  /**
   * @brief 获取大对象处理程序
   * 
   * @return LobFileHandler* 返回大对象处理程序
   */
  LobFileHandler *lob_handler() const { return lob_handler_; }

  /**
   * @brief 同步表数据到磁盘
   * 
   * 该方法将表的内存数据同步到磁盘，确保数据的持久性。
   * 
   * @return RC 返回状态码，SUCCESS表示同步成功，其他值表示同步失败
   */
  RC sync();

private:
  /**
   * @brief 将值设置到记录中
   * 
   * 该方法将指定的值按照字段的类型和长度设置到记录的对应位置。
   * 
   * @param record_data 记录数据
   * @param value 值
   * @param field 字段元数据
   * @return RC 返回状态码，SUCCESS表示设置成功，其他值表示设置失败
   */
  RC set_value_to_record(char *record_data, const Value &value, const FieldMeta *field);

private:
  // RC init_record_handler(const char *base_dir);

public:
  /**
   * @brief 根据索引名查找索引
   * 
   * 该方法根据索引名查找对应的索引对象。
   * 
   * @param index_name 索引名
   * @return Index* 返回索引指针，若找不到则返回nullptr
   */
  Index *find_index(const char *index_name) const;
  
  /**
   * @brief 根据字段名查找索引
   * 
   * 该方法根据字段名查找对应的索引对象。
   * 
   * @param field_name 字段名
   * @return Index* 返回索引指针，若找不到则返回nullptr
   */
  Index *find_index_by_field(const char *field_name) const;

private:
  Db       *db_ = nullptr;                      ///< 表所属的数据库对象
  TableMeta table_meta_;                       ///< 表的元数据
  // DiskBufferPool    *data_buffer_pool_ = nullptr;  /// 数据文件关联的buffer pool
  // RecordFileHandler *record_handler_   = nullptr;  /// 记录操作
  // vector<Index *>    indexes_;
  unique_ptr<TableEngine> engine_;             ///< 表引擎，负责表的实际存储和操作
  LobFileHandler         *lob_handler_ = nullptr; ///< 大对象处理程序
};
