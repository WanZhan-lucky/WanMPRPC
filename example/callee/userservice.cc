#include <iostream>
#include <string>

#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "user.pb.h"

// CMakeLists中存在搜索路径example
/*
UserService本地服务，提供两个进程内的本地方法， Login和GetFriendLists
*/

class UserService
    : public fixbug::UserServiceRpc {  // 使用在rpc服务发布端(rpc服务提供者)
 public:
  bool Login(std::string name, std::string pwd) {
    std::cout << "doing local service: Login" << std::endl;
    std::cout << "name:" << name << "pwd:" << pwd << std::endl;
    return true;
  }

  bool Register(uint32_t id, std::string name, std::string pwd) {
    std::cout << "doing local service: Register" << std::endl;
    std::cout << "id: " << id << "name: " << name << "pwd: " << pwd
              << std::endl;
    return true;
  }
  /*
  1. caller ---> Login(LoginRequest) => muduo  ---> callee
  2. callee ----> Login(LoginRequest) => 交到重写的Login方法
  */
  // 重写基类UserServiceRpc的虚函数， 下面方法都是框架直接调用
  void Login(::google::protobuf::RpcController* controller,
             const ::fixbug::LoginRequest* request,
             ::fixbug::LoginResponse* response,
             ::google::protobuf::Closure* done) {
    // 框架给业务上报了请求参数LoginRequest,
    // 业务获取相应数据，做本地业务（已经反序列化）
    std::string name = request->name();
    std::string pwd = request->pwd();

    // 作本地业务
    bool login_result = Login(name, pwd);

    // 把响应写入,
    fixbug::ResultCode* code = response->mutable_result();
    code->set_errcode(0);
    code->set_errmsg("");
    response->set_success(login_result);

    // 执行回调操作，框架给了request， reponse, 框架来做序列化和反序列化
    // 执行响应对象数据的序列化和网络发送（都是由框架来完成）
    done->Run();  // 需要重写，lamabad表达式
                  // 执行回调，直接
    /*
    google::protobuf::Closure *done =
      google::protobuf::NewCallback<RpcProvider,
                                    const muduo::net::TcpConnectionPtr &,
                                    google::protobuf::Message *>(
          this, &RpcProvider::SendRpcResponse, conn, response);*/
  }

  void Register(::google::protobuf::RpcController* controller,
                const ::fixbug::RegisterRequest* request,
                ::fixbug::RegisterResponse* response,
                ::google::protobuf::Closure* done) {
    uint32_t id = request->id();
    std::string name = request->name();
    std::string pwd = request->pwd();

    bool ret = Register(id, name, pwd);

    fixbug::ResultCode* resCode = response->mutable_result();

    resCode->set_errcode(0);
    resCode->set_errmsg("");
    response->set_success(ret);

    done->Run();  // 进行序列化，通过网络发送出去
  }
};

// int main() {
//   UserService us;
//   us.Login("xx", "yy");
//   return 0;
//   // 本地进程调用，没问题
// }

// 远程进程如何调用这个进程？

int main(int argc, char** argv) {
  // 调用框架的初始化
  MprpcApplication::Init(argc, argv);

  // provier时一个rpc网络服务对象，把userService对象发布到rpc节点上

  RpcProvider provider;
  provider.NotifyService(new UserService());
  // provider.notifyService(new ProductService());//也可以是一个商品服务
  // 启动一个rpc服务发布节点， Run以后，进程进入阻塞，等待远程的rpc调用请求
  provider.Run();
  return 0;
}