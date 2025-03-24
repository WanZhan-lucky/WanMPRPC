
#include "logger.h"

#include <time.h>

#include <iostream>
void Logger::SetLogLevel(LogLevel level) {
  m_loglevel = level;
}  // 设置日志级别
void Logger::WriteLogBuffer(std::string msg) {
  // 把日志写入到缓存区中
  m_lockQue.Push(msg);

}  // 写日志
// 获取日志单例
Logger& Logger::getInstance() {
  static Logger logger;
  return logger;
}

Logger::Logger() {
  // 启动后台写入磁盘的日志线程
  std::thread writeLogTask([&]() {
    for (;;) {
      // 获取当天的日期，取日志信息，写入响应的日志文件中
      // 追加方式!!!

      time_t now = time(nullptr);
      tm* nowtm = localtime(&now);

      char file_name[128];
      sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900,
              nowtm->tm_mon + 1, nowtm->tm_mday);

      FILE* pf = fopen(file_name, "a+");
      if (pf == nullptr) {
        std::cout << "logger file: " << file_name << "open error" << std::endl;
        exit(EXIT_FAILURE);
      }

      std::string msg = m_lockQue.Pop();

      char time_buf[128] = {0};
      // 时分秒
      sprintf(time_buf, "%d:%d:%d => [%s]", nowtm->tm_hour, nowtm->tm_min,
              nowtm->tm_sec, (m_loglevel == INFO ? "info" : "error"));
      msg.insert(0, time_buf);
      msg.append("\n");

      fputs(msg.c_str(), pf);
      fclose(pf);
    }
  });
  // 守护线程，分离线程
  writeLogTask.detach();
}
