#include "rpcprovider.h"

#include "logger.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"

void RpcProvider::NotifyService(google::protobuf::Service *service) {
  ServiceInfo service_info;

  // 获取服务对象的信息
  const google::protobuf::ServiceDescriptor *pserviceDesc =
      service->GetDescriptor();
  // 获取服务的名字
  std::string service_name = pserviceDesc->name();
  // std::cout << "service_name: " << service_name << std::endl;
  LOG_INFO("service_name: %s", service_name.c_str());
  // 获取方法数量
  int methodCnt = pserviceDesc->method_count();

  for (int i = 0; i < methodCnt; i++) {
    // 获取服务对象指定下标的服务方法的描述（抽象描述）
    const google::protobuf::MethodDescriptor *pmethodDesc =
        pserviceDesc->method(i);

    std::string method_name = pmethodDesc->name();
    service_info.m_methodMap.insert({method_name, pmethodDesc});

    // std::cout << "method_name: " << method_name << std::endl;
    LOG_INFO("method: %s", method_name.c_str());
  }

  service_info.m_service = service;
  m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run() {
  std::string ip =
      MprpcApplication::getInstance().GetConfig().Load("rpcserverip");
  uint16_t port = atoi(MprpcApplication::getInstance()
                           .GetConfig()
                           .Load("rpcserverport")
                           .c_str());

  muduo::net::InetAddress address(ip, port);

  // 创建TcpServer对象
  muduo::net::TcpServer server(&m_eventloop, address, "RpcProvider");
  // 绑定连接回调和消息读写回调方法， 分离了网络代码和业务代码
  server.setConnectionCallback(
      std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
  server.setMessageCallback(
      std::bind(&RpcProvider::onMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  // 设置muduo库的线程数量
  server.setThreadNum(4);

  // 把当前rpc节点上要发布的服务全部注册到zk上卖弄，让rpc
  // client可以从zk上发现服务
  //  把当前rpc节点上要发布的服务全部注册到zk上面，让rpc
  //  client可以从zk上发现服务 session timeout   30s     zkclient 网络I/O线程
  //  1/3 * timeout 时间发送ping消息
  ZkClient zkCli;
  zkCli.Start();
  // service_name为永久性节点    method_name为临时性节点
  for (auto &sp : m_serviceMap) {
    // /service_name   /UserServiceRpc
    std::string service_path = "/" + sp.first;
    zkCli.Create(service_path.c_str(), nullptr, 0);
    for (auto &mp : sp.second.m_methodMap) {
      // /service_name/method_name   /UserServiceRpc/Login
      // 存储当前这个rpc服务节点主机的ip和port
      std::string method_path = service_path + "/" + mp.first;
      char method_path_data[128] = {0};
      sprintf(method_path_data, "%s:%d", ip.c_str(), port);
      // ZOO_EPHEMERAL表示znode是一个临时性节点
      zkCli.Create(method_path.c_str(), method_path_data,
                   strlen(method_path_data), ZOO_EPHEMERAL);
    }
  }

  // rpc服务启动，打印ip和port
  std::cout << "RpcProvider start service at ip: " << ip << " port: " << port
            << std::endl;

  // 启动网络服务
  server.start();
  m_eventloop.loop();
}

// 新socket连接回调, 三次握手，四次挥手
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn) {
  if (!conn->connected()) {
    conn->shutdown();
  }
}

// 框架内部，prcRpovider和rpcconsumer协商好两者之间通信的的portobuf数据类型
// 定义proto的message类型的，进行数据的序列化和反序列化
// service_name method_name args_size

// 16UserServiceLoginzhangsan123456  例子

// header_size + header_str + args_str
// headers_size获取后，获取header_str, header_str包含service_name,
// method_name和args_size
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp timestamp) {
  // 网络上接受的远程rpc调用请求的字符流
  std::string recv_buf = buffer->retrieveAllAsString();

  // 从字符流中读取前4个字节内容
  uint32_t header_size = 0;
  recv_buf.copy((char *)&header_size, 4, 0);
  // copy必须是char类型指针，4为字节数，0为起始位置

  // 根据header_size读取数据头的原始字符流,反序列化数据，得到rpc请求的详细信息
  std::string rpc_header_str = recv_buf.substr(4, header_size);

  mprpc::RpcHeader rpcHeader;
  std::string service_name;
  std::string method_name;
  uint32_t args_size;
  if (rpcHeader.ParseFromString(rpc_header_str)) {
    // 数据头反序列化成功
    service_name = rpcHeader.service_name();
    method_name = rpcHeader.method_name();
    args_size = rpcHeader.args_size();
  } else {
    // 数据头反序列失败
    std::cout << "rpc_header_str:" << rpc_header_str << "parse error!"
              << std::endl;
    return;
  }
  // 获取rpc方法参数的字符流数据
  std::string args_str = recv_buf.substr(4 + header_size, args_size);

  // 要知道哪个服务的哪个方法以及哪个方法
  //  打印调式
  std::cout << "-------------------------------" << std::endl;
  std::cout << "provider-header_size: " << header_size << std::endl;
  std::cout << "provider-rpc_header_str: " << rpc_header_str << std::endl;
  std::cout << "provider-service_name: " << service_name << std::endl;
  std::cout << "provider-method_name: " << method_name << std::endl;
  std::cout << "provider-args_str: " << args_str << std::endl;
  std::cout << "-------------------------------" << std::endl;

  // 获取service对象和method对象

  auto it = m_serviceMap.find(service_name);
  if (it == m_serviceMap.end()) {  // 服务对象不存在
    std::cout << service_name << " is not exists" << std::endl;
    return;
  }

  auto mit = it->second.m_methodMap.find(method_name);
  if (mit == it->second.m_methodMap.end()) {  // 方法不存在
    std::cout << service_name << ":" << method_name << " is not exists"
              << std::endl;
  }

  // 获取service对象  new UserService
  google::protobuf::Service *service = it->second.m_service;
  // 获取method对象  Login方法
  const google::protobuf::MethodDescriptor *method = mit->second;

  // 生成rpc方法调用的请求request和响应reponse参数
  google::protobuf::Message *request =
      service->GetRequestPrototype(method).New();
  if (!request->ParseFromString(args_str)) {  // 添加到request中
    std::cout << "request parse error! content: " << args_str << std::endl;
    return;
  }
  // response由业务来填
  google::protobuf::Message *response =
      service->GetResponsePrototype(method).New();

  // 给下面的method方法的调用，绑定一个Closure的回调函数
  google::protobuf::Closure *done =
      google::protobuf::NewCallback<RpcProvider,
                                    const muduo::net::TcpConnectionPtr &,
                                    google::protobuf::Message *>(
          this, &RpcProvider::SendRpcResponse, conn, response);

  // 在框架上
  // 在框架上根据远端rpc请求，调用当前rpc节点上的发布方法
  // new UserService().Login(controller, request, response, done);
  service->CallMethod(method, nullptr, request, response,
                      done);  // 调用UserService中重写的Login服务方法
                              // CallMethod通过method找到Login方法
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn,
                                  google::protobuf::Message *response) {
  std::string response_str;
  if (response->SerializeToString(&response_str)) {
    // response进行序列化
    // 序列化成功后，通过网络把rpc方法执行的结果发送给rpc调用方
    conn->send(response_str);
    conn->shutdown();  // 模拟http的短链接服务，由rpcprovider主动断开连接
  } else {
    std::cout << "serialize response_str error!" << std::endl;
  }
  conn->shutdown();
}