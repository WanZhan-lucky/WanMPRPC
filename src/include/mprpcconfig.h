#pragma once

#include <string>
#include <unordered_map>

class MprpcConfig {
 public:
  // 解译加载配置文件
  void LoadConfigFile(const char* config_file);
  // 查询配置项信息
  std::string Load(const std::string& key);

 private:
  // 框架只用启动一次，无需进行线程安全
  std::unordered_map<std::string, std::string> m_configMap;
  // 去掉字符串前后空格
  void Trim(std::string& str_buf);
};