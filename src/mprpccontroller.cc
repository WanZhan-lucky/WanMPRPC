#include "mprpccontroller.h"

MprpcContorller::MprpcContorller() {
  m_failed = false;
  m_errText = "";
}
void MprpcContorller::Reset() {
  m_failed = false;
  m_errText = "";
}
bool MprpcContorller::Failed() const { return m_failed; }
std::string MprpcContorller::ErrorText() const { return m_errText; }
void MprpcContorller::SetFailed(const std::string& reason) {
  m_failed = true;
  m_errText = reason;
}

// 目前未实现具体的功能
void MprpcContorller::StartCancel() {}
// 默认返回false
bool MprpcContorller::IsCanceled() const { return false; }
void MprpcContorller::NotifyOnCancel(google::protobuf::Closure* callback) {}
