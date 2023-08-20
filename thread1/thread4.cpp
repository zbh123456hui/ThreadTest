#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <list>
#include <vector>

template <typename T>
class BlockingQueue {
 public:
  BlockingQueue(uint size) : size_(size) {}
  BlockingQueue() {}

  /**
   * @brief 向队列中push_back一个元素
   * **/
  void put(const T x) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (size_ != 0 && queue_.size() > size_) {
      queue_.pop_front();
    }
    queue_.push_back(std::move(x));
    not_empty_.notify_all();  //有新数据push进来后就唤醒所有wait的线程
  }

  /**
   * @brief 获取队列中第一个元素（不是最新元素）
   * 元素一旦获取，就从队列中删除
   * **/
  T take() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      not_empty_.wait(lock);  //如果队列为空，线程就在此阻塞挂起，等待唤醒
    }
    const T front = queue_.front();
    queue_.pop_front();
    return front;
  }

  /**
   * @brief 取出队列中所有元素(取出后就从队列中删除),并push_back到一个vector中
   * **/
  int take(std::vector<T> *v, size_t count) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      not_empty_.wait(lock);  //如果队列为空，线程就在此阻塞挂起，等待唤醒
    }
    while (!queue_.empty() && count--) {
      v->push_back(queue_.front());
      queue_.pop_front();
    }

    return v->size();
  }

  /**
   * @brief 根据索引获取队列中对应元素
   * **/
  T get(size_t index) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      not_empty_.wait(lock);
    }
    return queue_.at(index);
  }

  /**
   * @brief 获取队列元素个数
   * **/
size_t size() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable not_empty_;
  std::deque<T> queue_;
  // std::list<T> queue_;//双向链表
  
  uint size_ = 0;
};
