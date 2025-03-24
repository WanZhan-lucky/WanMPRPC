#pragma once
#include "mprpcchannel.h"
#include "mprpcconfig.h"
#include "mprpccontroller.h"

// mprpc 框架基础类，初始化类， 初始化操作，单例模式
class MprpcApplication {
 public:
  static void Init(int argc, char **argv);
  static MprpcApplication &getInstance();
  static MprpcConfig &GetConfig();

 private:
  MprpcApplication() {};
  MprpcApplication(const MprpcApplication &) = delete;
  MprpcApplication(MprpcApplication &&) = delete;

  static MprpcConfig m_config;  // 静态变量需要类外初始化，那么直接{}
};