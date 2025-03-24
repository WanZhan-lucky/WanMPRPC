#include <iostream>
#include <string>
#include <vector>

#include "friends.pb.h"
#include "logger.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

class wanFriendServer : public fixbug::FriendsServiceWanRpc {
 public:
  std::vector<std::string> getFriendswan(uint32_t userid) {
    std::cout << "do getFriendswan service , userid:" << userid << std::endl;

    std::vector<std::string> res;
    res.push_back("wan");
    res.push_back("zhi");
    res.push_back("zhan");

    return res;
  }

  void GetFriendLists(::google::protobuf::RpcController* controller,
                      const ::fixbug::GetFriendsRequest* request,
                      ::fixbug::GetFriendsResponse* response,
                      ::google::protobuf::Closure* done) {
    uint16_t usrid = request->id();

    std::vector<std::string> Resfriends = getFriendswan(usrid);

    response->mutable_result()->set_errncode(0);
    response->mutable_result()->set_errmsg("");
    for (std::string& friendname : Resfriends) {
      std::string* p = response->add_friends();
      *p = friendname;
    }
    done->Run();
  }
};

int main(int argc, char** argv) {
  // 记录日志信息
  LOG_INFO("first log message!");
  LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

  // 调用框架的初始化
  MprpcApplication::Init(argc, argv);

  // provier时一个rpc网络服务对象，把userService对象发布到rpc节点上

  RpcProvider provider;
  provider.NotifyService(new wanFriendServer());
  // provider.notifyService(new ProductService());//也可以是一个商品服务
  // 启动一个rpc服务发布节点， Run以后，进程进入阻塞，等待远程的rpc调用请求
  provider.Run();
  return 0;
}