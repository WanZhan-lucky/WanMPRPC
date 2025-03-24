#pragma once
#include <google/protobuf/descriptor.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/TcpServer.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "google/protobuf/service.h"
#include "mprpcapplication.h"
// 框架提供的专门服务发布服务rpc服务的网络对象

class RpcProvider {
 public:
  // 这是框架提供外部使用的，可以发布rpc方法的函数接口，接受任意的service
  void NotifyService(google::protobuf::Service *service);
  // 启动rpc服务节点，开始提供rpc远程网络调用服务
  void Run();

 private:
  // // 组合了TCPServer
  // std::unique_ptr<muduo::net::TcpServer> m_tcpserverPtr;
  // 组合了EventLoop
  muduo::net::EventLoop m_eventloop;

  // service服务类型信息
  struct ServiceInfo {
    // 保存服务对象
    google::protobuf::Service *m_service;
    // 保存服务方法
    std::unordered_map<std::string, const google::protobuf::MethodDescriptor *>
        m_methodMap;
  };
  // 存储注册的服务对象和其他服务方法的所有信息
  std::unordered_map<std::string, ServiceInfo> m_serviceMap;

  // 新的socket连接回调
  void onConnection(const muduo::net::TcpConnectionPtr &);  // shared_ptr

  // 已建立连接用户的读写事件回调
  void onMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *,
                 muduo::Timestamp);

  // Closure的回调操作，用于序列化rpc的响应和网络发送
  void SendRpcResponse(const muduo::net::TcpConnectionPtr &,
                       google::protobuf::Message *);
};