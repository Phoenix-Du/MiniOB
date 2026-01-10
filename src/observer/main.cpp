/**
 * @file main.cpp
 * @brief MiniOB数据库服务器主入口文件
 * 
 * 该文件包含MiniOB数据库服务器的启动、配置和主循环逻辑，是整个系统的入口点。
 * 主要功能包括：
 * - 解析命令行参数
 * - 初始化服务器环境
 * - 创建并启动网络服务器
 * - 处理信号和优雅关闭
 * 
 * @author Longda Feng
 * @date Mar 11, 2012
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

#include <netinet/in.h>
#include <unistd.h>

#include "common/ini_setting.h"
#include "common/init.h"
#include "common/lang/iostream.h"
#include "common/lang/string.h"
#include "common/lang/map.h"
#include "common/os/process.h"
#include "common/os/signal.h"
#include "common/log/log.h"
#include "net/server.h"
#include "net/server_param.h"

using namespace common;

#define NET "NET"  ///< 网络配置部分的名称

static Server *g_server = nullptr;  ///< 全局服务器实例指针

/**
 * @brief 显示服务器的命令行参数使用说明
 * 
 * 当用户输入-h参数或错误参数时，该函数会显示所有支持的命令行选项及其详细说明。
 */
void usage()
{
  cout << "Usage " << endl;
  cout << "-p: server port. if not specified, the item in the config file will be used" << endl;
  cout << "-f: path of config file." << endl;
  cout << "-s: use unix socket and the argument is socket address" << endl;
  cout << "-P: protocol. {plain(default), mysql, cli}." << endl;
  cout << "-t: transaction model. {vacuous(default), mvcc}." << endl;
  cout << "-T: thread handling model. {one-thread-per-connection(default),java-thread-pool}." << endl;
  cout << "-n: buffer pool memory size in byte" << endl;
  cout << "-d: durbility mode. {vacuous(default), disk}" << endl;
  // TODO: support multi dbs(storage/db/db.h) and remove this options
  cout << "-E: storage engine. {heap(default), lsm}" << endl;
}

/**
 * @brief 解析命令行参数
 * 
 * 该函数负责解析用户传入的命令行参数，并将参数值设置到全局的ProcessParam对象中。
 * 支持的参数包括端口号、配置文件路径、协议类型、事务模型等。
 * 
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 */
void parse_parameter(int argc, char **argv)
{
  string process_name = get_process_name(argv[0]);

  ProcessParam *process_param = the_process_param();

  process_param->init_default(process_name);

  // Process args
  int          opt;
  extern char *optarg;
  while ((opt = getopt(argc, argv, "dp:P:s:t:T:f:o:e:E:hn:")) > 0) {
    switch (opt) {
      case 's': process_param->set_unix_socket_path(optarg); break;  ///< 设置Unix socket路径
      case 'p': process_param->set_server_port(atoi(optarg)); break;  ///< 设置服务器端口
      case 'P': process_param->set_protocol(optarg); break;  ///< 设置通信协议
      case 'f': process_param->set_conf(optarg); break;  ///< 设置配置文件路径
      case 'o': process_param->set_std_out(optarg); break;  ///< 设置标准输出文件
      case 'e': process_param->set_std_err(optarg); break;  ///< 设置标准错误文件
      case 't': process_param->set_trx_kit_name(optarg); break;  ///< 设置事务模型
      case 'E': process_param->set_storage_engine(optarg); break;  ///< 设置存储引擎
      case 'T': process_param->set_thread_handling_name(optarg); break;  ///< 设置线程处理模型
      case 'n': process_param->set_buffer_pool_memory_size(atoi(optarg)); break;  ///< 设置缓冲池大小
      case 'd': process_param->set_durability_mode("disk"); break;  ///< 设置持久化模式为磁盘
      case 'h':
        usage();  ///< 显示使用说明
        exit(0);
        return;
      default: cout << "Unknown option: " << static_cast<char>(opt) << ", ignored" << endl; break;
    }
  }
}

/**
 * @brief 初始化服务器实例
 * 
 * 该函数根据配置文件和命令行参数创建并初始化服务器实例。
 * 主要工作包括：
 * - 从配置文件读取网络配置
 * - 合并命令行参数中的配置
 * - 创建ServerParam对象
 * - 根据参数创建不同类型的服务器（CliServer或NetServer）
 * 
 * @return Server* 创建的服务器实例指针
 */
Server *init_server()
{
  // 获取网络配置部分
  map<string, string> net_section = get_properties()->get(NET);

  ProcessParam *process_param = the_process_param();

  long listen_addr        = INADDR_ANY;  ///< 默认监听所有地址
  long max_connection_num = MAX_CONNECTION_NUM_DEFAULT;  ///< 默认最大连接数
  int  port               = PORT_DEFAULT;  ///< 默认端口

  // 从配置文件读取监听地址
  map<string, string>::iterator it = net_section.find(CLIENT_ADDRESS);
  if (it != net_section.end()) {
    string str = it->second;
    str_to_val(str, listen_addr);
  }

  // 从配置文件读取最大连接数
  it = net_section.find(MAX_CONNECTION_NUM);
  if (it != net_section.end()) {
    string str = it->second;
    str_to_val(str, max_connection_num);
  }

  // 确定端口号（命令行参数优先于配置文件）
  if (process_param->get_server_port() > 0) {
    port = process_param->get_server_port();
    LOG_INFO("Use port config in command line: %d", port);
  } else {
    it = net_section.find(PORT);
    if (it != net_section.end()) {
      string str = it->second;
      str_to_val(str, port);
    }
  }

  // 创建服务器参数对象
  ServerParam server_param;
  server_param.listen_addr        = listen_addr;
  server_param.max_connection_num = max_connection_num;
  server_param.port               = port;
  
  // 设置通信协议
  if (0 == strcasecmp(process_param->get_protocol().c_str(), "mysql")) {
    server_param.protocol = CommunicateProtocol::MYSQL;
  } else if (0 == strcasecmp(process_param->get_protocol().c_str(), "cli")) {
    server_param.use_std_io = true;
    server_param.protocol   = CommunicateProtocol::CLI;
  } else {
    server_param.protocol = CommunicateProtocol::PLAIN;
  }

  // 设置Unix socket（如果需要）
  if (process_param->get_unix_socket_path().size() > 0 && !server_param.use_std_io) {
    server_param.use_unix_socket  = true;
    server_param.unix_socket_path = process_param->get_unix_socket_path();
  }
  
  // 设置线程处理模型
  server_param.thread_handling = process_param->thread_handling_name();

  // 创建服务器实例
  Server *server = nullptr;
  if (server_param.use_std_io) {
    server = new CliServer(server_param);  ///< 创建命令行服务器
  } else {
    server = new NetServer(server_param);  ///< 创建网络服务器
  }

  return server;
}

/**
 * @brief 退出处理线程函数
 * 
 * 该函数在信号处理线程中执行，用于优雅地关闭服务器。
 * 单独创建线程的原因是避免在信号处理函数中执行可能导致死锁的操作（如日志记录）。
 * 
 * @param _signum 收到的信号编号
 * @return void* 线程返回值
 */
void *quit_thread_func(void *_signum)
{
  intptr_t signum = (intptr_t)_signum;
  LOG_INFO("Receive signal: %ld", signum);
  if (g_server) {
    g_server->shutdown();  ///< 关闭服务器
  }
  return nullptr;
}

/**
 * @brief 信号处理函数
 * 
 * 该函数处理中断信号（如Ctrl+C），用于启动优雅关闭流程。
 * 它会重置信号处理函数以防止多次调用，然后创建一个新线程来执行实际的关闭操作。
 * 
 * @param signum 收到的信号编号
 */
void quit_signal_handle(int signum)
{
  // 防止多次调用退出
  set_signal_handler(nullptr);

  pthread_t tid;
  pthread_create(&tid, nullptr, quit_thread_func, (void *)(intptr_t)signum);  ///< 创建退出处理线程
}

/**
 * @brief 服务器启动提示信息
 */
const char *startup_tips = R"(
Welcome to the OceanBase database implementation course.

Copyright (c) 2021 OceanBase and/or its affiliates.

Learn more about OceanBase at https://github.com/oceanbase/oceanbase
Learn more about MiniOB at https://github.com/oceanbase/miniob

)";

/**
 * @brief 数据库服务器的主函数入口
 * 
 * 该函数是MiniOB数据库服务器的启动点，负责完成以下工作：
 * 1. 显示启动信息
 * 2. 设置信号处理函数，用于优雅关闭服务器
 * 3. 解析命令行参数
 * 4. 初始化服务器环境
 * 5. 创建并启动服务器实例
 * 6. 等待服务器停止
 * 7. 清理资源并退出
 * 
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return int 程序退出状态码，0表示成功，非0表示失败
 */
int main(int argc, char **argv)
{
  int rc = STATUS_SUCCESS;  ///< 返回状态码，默认成功

  // 显示启动信息
  cout << startup_tips;

  // 设置信号处理函数
  set_signal_handler(quit_signal_handle);

  // 解析命令行参数
  parse_parameter(argc, argv);

  // 初始化服务器环境
  rc = init(the_process_param());
  if (rc != STATUS_SUCCESS) {
    cerr << "Shutdown due to failed to init!" << endl;
    cleanup();  ///< 清理资源
    return rc;
  }

  // 创建并初始化服务器实例
  g_server = init_server();
  
  // 启动服务器，开始监听和处理客户端请求
  g_server->serve();

  LOG_INFO("Server stopped");

  // 清理资源
  cleanup();

  // 删除服务器实例
  delete g_server;
  return 0;
}
