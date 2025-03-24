#include <iostream>

#include "mprpcapplication.h"
#include "mprpcchannel.h"
#include "user.pb.h"
// 业务代码

int main(int argc, char **argv) {
  // 整个程序启动以后，使用mprpc框架来享受rpc服务调用，一定先用框架的初始化函数，只初始化一次
  MprpcApplication::Init(argc, argv);
  // 演示调用远程发布的rpc方法Login
  fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

  // rpc方法的请求
  fixbug::LoginRequest request;
  request.set_name("wanzhan");
  request.set_pwd("123456");
  // rpc方法的响应
  fixbug::LoginResponse response;

  // controller， 和done指为空即可
  // 发起rpc方法的调用，同步的rpc调用过程，同步阻塞
  stub.Login(nullptr, &request, &response,
             nullptr);  // RpcChannel->RpcChannel::callMethod
  // 集中所有的rpc方法来调用的参数序列化和网络发送

  // 一次调用完成，读调用结果
  if (0 == response.result().errcode()) {
    std::cout << "rpc login response success: " << response.success()
              << std::endl;
  } else {
    std::cout << "rpc login response error: " << response.result().errmsg()
              << std::endl;
  }

  // rpc方法的注册
  fixbug::RegisterRequest req;
  req.set_id(100);
  req.set_name("wanzhandenghuo");
  req.set_pwd("123123");
  // rpc方法的响应
  fixbug::RegisterResponse rsp;

  // controller， 和done指为空即可
  // 发起rpc方法的调用，同步的rpc调用过程，同步阻塞
  stub.Register(nullptr, &req, &rsp,
                nullptr);  // RpcChannel->RpcChannel::callMethod
  // 集中所有的rpc方法来调用的参数序列化和网络发送

  // 一次调用完成，读调用结果
  if (0 == rsp.result().errcode()) {
    std::cout << "rpc register response success: " << response.success()
              << std::endl;
  } else {
    std::cout << "rpc register response error: " << response.result().errmsg()
              << std::endl;
  }

  return 0;
}