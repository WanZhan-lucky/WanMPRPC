#pragma once
#include <condition_variable>  //pthread_condition_t通信，同步通信
#include <mutex>               //互斥锁
#include <queue>
#include <thread>

// 异步写日志的日志队列
template <typename T>
class LockQueue {
 public:
  // 多个work线程写日志
  void Push(const T& data) {
    std::lock_guard<std::mutex> lock(m_mutext);
    m_queue.push(data);
    m_convariable.notify_one();
  }
  T Pop() {  // 磁盘io线程读日志，写日志
    std::unique_lock<std::mutex> lock(m_mutext);
    while (m_queue.empty()) {
      // 日志队列为空，线程进入wait等待
      m_convariable.wait(lock);
    }
    T data = m_queue.front();
    m_queue.pop();
    return data;
  }

 private:
  std::queue<T> m_queue;
  std::mutex m_mutext;
  std::condition_variable m_convariable;
};
