#include <iostream>
#include <string>

#include "test.pb.h"
using namespace fixbug;
int main() {
  LoginResponse rsp;
  ResultCode *rc = rsp.mutable_result();
  rc->set_errcode(1);
  rc->set_errmsg("登录处理失败了");

  GetFriendListsResponse rps;
  ResultCode *rt = rsp.mutable_result();
  rt->set_errcode(0);

  User *u1 = rps.add_friend_list();
  u1->set_name("wanm");
  u1->set_age(20);
  u1->set_sex(User::MAN);

  User *u2 = rps.add_friend_list();
  u2->set_name("wanmfsdaf");
  u2->set_age(223);
  u2->set_sex(User::WOMAN);

  std::cout << rps.friend_list_size() << std::endl;
  return 0;
}

int main2() {
  // 封装login请求对象数据
  LoginRequest req;
  req.set_name("zhangfdfsafsan");
  req.set_pwd("123456");
  // 独享数据序列化
  std::string send_str;
  if (req.SerializePartialToString(&send_str)) {
    std::cout << send_str << std::endl;
    std::cout << send_str.c_str() << std::endl;
  }

  // 反序列化
  LoginRequest reqb;
  if (reqb.ParseFromString(send_str)) {
    std::cout << reqb.name() << std::endl;
    std::cout << reqb.pwd() << std::endl;
  }

  return 0;
}