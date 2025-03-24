#include <iostream>

#include "friends.pb.h"
#include "mprpcapplication.h"
// 用户引用越简单越好

int main(int argc, char **argv) {
  // 整个程序启动以后，使用mprpc框架来享受rpc服务调用，一定先用框架的初始化函数，只初始化一次
  MprpcApplication::Init(argc, argv);
  // 演示调用远程发布的rpc方法Login
  fixbug::FriendsServiceWanRpc_Stub stub(new MprpcChannel());

  // rpc方法的请求
  fixbug::GetFriendsRequest request;
  request.set_id(100);

  // rpc方法的响应
  fixbug::GetFriendsResponse response;

  // controller， 和done指为空即可
  // 发起rpc方法的调用，同步的rpc调用过程，同步阻塞
  MprpcContorller controller;
  stub.GetFriendLists(&controller, &request, &response,
                      nullptr);  // RpcChannel->RpcChannel::callMethod
  // 集中所有的rpc方法来调用的参数序列化和网络发送

  // 出错了直接就不解析，只要一个出错，网络发送...
  if (controller.Failed()) {
    std::cout << controller.ErrorText() << std::endl;
  } else {
    // 一次调用完成，读调用结果
    if (0 == response.result().errncode()) {
      std::cout << "rpc friends response success: " << std::endl;
      int size = response.friends_size();
      for (int i = 0; i < size; i++) {
        std::cout << "index: " << i << " name: " << response.friends(i)
                  << std::endl;
      }

    } else {
      std::cout << "rpc friends response error: " << response.result().errmsg()
                << std::endl;
    }
  }

  return 0;
}