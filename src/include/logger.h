#pragma once
#include "lockqueue.h"

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
#define LOG_INFO(logmsgformat, ...)                 \
  do {                                              \
    Logger& logger = Logger::getInstance();         \
    logger.SetLogLevel(INFO);                       \
    char c[1024] = {0};                             \
    snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
    logger.WriteLogBuffer(c);                       \
  } while (0)

#define LOG_ERR(logmsgformat, ...)                  \
  do {                                              \
    Logger& logger = Logger::getInstance();         \
    logger.SetLogLevel(ERROR);                      \
    char c[1024] = {0};                             \
    snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
    logger.WriteLogBuffer(c);                       \
  } while (0)

// 框架的日志系统
enum LogLevel {
  INFO,  // 普通日志信息
  ERROR  // 错误信息
};

class Logger {
 public:
  void SetLogLevel(LogLevel level);      // 设置日志级别
  void WriteLogBuffer(std::string msg);  // 写日志
  // 获取日志单例
  static Logger& getInstance();

 private:
  int m_loglevel;                    // 记录日志级别
  LockQueue<std::string> m_lockQue;  // 日志缓存队列

  Logger();
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
};

// 定义宏
// #define LOG_INFO(logmsgformat, ...)                 \
//   do {                                              \
//     Logger& logger = Logger::getInstance();         \
//     logger.SetLogLevel(INFO);                       \
//     char c[1024] = {0};                             \
//     snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
//     logger.WriteLogBuffer(c);                       \
//   } while (0);

// #define LOG_ERR(logmsgformat, ...)                  \
//   do {                                              \
//     Logger& logger = Logger::getInstance();         \
//     logger.SetLogLevel(ERROR);                      \
//     char c[1024] = {0};                             \
//     snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
//     logger.WriteLogBuffer(c);                       \
//   } while (0);
