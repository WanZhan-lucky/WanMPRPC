#include "mprpcchannel.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"
// header_size + service_name method_name  args_size + args
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                              google::protobuf::RpcController* controller,
                              const google::protobuf::Message* request,
                              google::protobuf::Message* response,
                              google::protobuf::Closure* done) {
  // rpc调用方， done没有用
  const google::protobuf::ServiceDescriptor* sd = method->service();
  std::string service_name = sd->name();
  std::string method_name = method->name();

  // 获取参数的序列化字符串长度 args_size
  uint32_t args_size = 0;
  std::string args_str;

  if (request->SerializeToString(&args_str)) {
    args_size = args_str.size();
    std::cout << "serialize request success" << std::endl;
  } else {
    // std::cout << "serialize request error!" << std::endl;
    controller->SetFailed("serialize request error!");
    return;
  }

  // 定义rpc的请求头
  mprpc::RpcHeader rpcHeader;
  rpcHeader.set_service_name(service_name);
  rpcHeader.set_method_name(method_name);
  rpcHeader.set_args_size(args_size);

  uint32_t header_size = 0;
  std::string rpc_header_str;
  if (rpcHeader.SerializeToString(&rpc_header_str)) {
    header_size = rpc_header_str.size();
    std::cout << "serialize rpc header success!" << std::endl;
  } else {
    // std::cout << "serialize rpc header error!" << std::endl;
    controller->SetFailed("serialize rpc header error!");
    return;
  }

  // 组织待发送的rpc请求的字符串
  std::string send_rpc_str;
  send_rpc_str.insert(0, std::string((char*)&header_size, 4));
  send_rpc_str += rpc_header_str;
  send_rpc_str += args_str;

  //  打印调式
  std::cout << "-------------------------------" << std::endl;
  std::cout << "consumer-header_size: " << header_size << std::endl;
  std::cout << "consumer-rpc_header_str: " << rpc_header_str << std::endl;
  std::cout << "consumer-service_name: " << service_name << std::endl;
  std::cout << "consumer-method_name: " << method_name << std::endl;
  std::cout << "consumer-args_str: " << args_str << std::endl;
  std::cout << "-------------------------------" << std::endl;

  // 使用TCP编程,完成rpc远程调用
  // 调用端， 简单TCP编程,即可，原始的TCP连接，不需要muduo来实现高并发
  int clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == clientfd) {
    // std::cout << "create socket errno: " << errno << "-" << strerror(errno)
    //           << std::endl;
    char errtxt[512] = {0};
    sprintf(errtxt, "create socket error!errno:%d", errno);
    controller->SetFailed(errtxt);
    return;
    // exit(EXIT_FAILURE);
  }

  // 读取配置文件rpcserver的信息

  // std::string ip =
  //     MprpcApplication::getInstance().GetConfig().Load("rpcserverip");
  // uint16_t port = atoi(MprpcApplication::getInstance()
  //                          .GetConfig()
  //                          .Load("rpcserverport")
  //                          .c_str());

  // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
  ZkClient zkCli;
  zkCli.Start();
  //  /UserServiceRpc/Login
  std::string method_path = "/" + service_name + "/" + method_name;
  // 127.0.0.1:8000
  std::string host_data = zkCli.GetData(method_path.c_str());
  if (host_data == "") {
    controller->SetFailed(method_path + " is not exist!");
    return;
  }
  int idx = host_data.find(":");
  if (idx == -1) {
    controller->SetFailed(method_path + " address is invalid!");
    return;
  }
  std::string ip = host_data.substr(0, idx);
  uint16_t port =
      atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

  // 连接rpc服务节点
  if (-1 ==
      connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
    // std::cout << "connect error errno: " << errno << "-" << strerror(errno)
    //           << std::endl;
    close(clientfd);

    char errtxt[512] = {0};
    sprintf(errtxt, "connect socket error!errno:%d", errno);
    controller->SetFailed(errtxt);
    return;
    // exit(EXIT_FAILURE);
  }
  // 发送rpc请求
  if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)) {
    // std::cout << "send error : " << errno << "-" << strerror(errno)
    //           << std::endl;
    close(clientfd);
    char errtxt[512] = {0};
    sprintf(errtxt, "send error!errno:%d", errno);
    controller->SetFailed(errtxt);
    return;
  }

  // 接受rpc请求的响应值
  char recv_buf[2014] = {0};
  int recv_size = 0;
  if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0))) {
    // std::cout << "recv error : " << errno << "-" << strerror(errno)
    //           << std::endl;
    close(clientfd);

    char errtxt[512] = {0};
    sprintf(errtxt, "recv error!errno:%d", errno);
    controller->SetFailed(errtxt);
    return;
  }

  // std::string response_str(recv_buf, 0, recv_size);
  // bug, recv_buf遇到\0后面的数据就存不下来
  // if (!response->ParseFromString(response_str)) {

  if (!response->ParseFromArray(recv_buf, recv_size)) {
    // std::cout << "parse error! response_str:" << response << std::endl;
    close(clientfd);

    char errtxt[2048] = {0};
    sprintf(errtxt, "parse error!errno:%d, recv_str: %s", errno, recv_buf);
    controller->SetFailed(errtxt);
    return;
  }

  close(clientfd);
  // 当然也可以采用智能指针，自动释放，自动关闭
}