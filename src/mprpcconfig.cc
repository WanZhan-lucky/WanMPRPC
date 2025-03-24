#include "mprpcconfig.h"

#include <iostream>
#include <string>
void MprpcConfig::LoadConfigFile(const char* config_file) {
  FILE* pf = fopen(config_file, "r");
  if (pf == nullptr) {
    std::cout << config_file << " is not exists" << std::endl;
    exit(EXIT_FAILURE);
  }

  while (!feof(pf)) {
    // 1注释 2正确的配置项 3，去掉开头的空格
    char buf[512] = {0};
    fgets(buf, 512, pf);

    // 去掉空格
    std::string read_buf(buf);
    Trim(read_buf);  // 取出前后中间空格

    // 判断#的注释
    if (read_buf[0] == '#' || read_buf.empty()) continue;

    // 配置解析项
    int idx = read_buf.find('=');
    if (idx == -1) {
      continue;
    }

    std::string key;
    std::string value;
    key = read_buf.substr(0, idx);
    Trim(key);

    int endidx = read_buf.find('\n', idx);
    value = read_buf.substr(idx + 1, endidx - idx - 1);
    Trim(value);
    m_configMap.insert({key, value});
  }
}

std::string MprpcConfig::Load(const std::string& key) {
  // return m_configMap[key]; 如果不存在,中括号会添加
  auto it = m_configMap.find(key);
  if (it != m_configMap.end()) return m_configMap[key];
  return "";
}

void MprpcConfig::Trim(std::string& src_buf) {
  int idx = src_buf.find_first_not_of(' ');
  if (idx != -1) {
    // 说明有空格
    src_buf = src_buf.substr(idx, src_buf.size() - idx);
  }
  idx = src_buf.find_last_not_of(' ');
  if (idx != -1) {
    // 去掉末尾空格
    src_buf = src_buf.substr(0, idx + 1);
  }
}