#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <unistd.h>
#include "BlockingQueue.h"

void BlockingQueue::enqueue(task_t task){
    std::unique_lock<std::mutex> enlock(enCounterMtx);   
    //wait()函数在条件为 false 时会阻塞当前线程，
    //在收到其他线程的通知后且为 true 时才会被解除阻塞
    repository_notFull.wait(enlock, [this] {
    bool full = m_queue.size() >= m_capacity;
    if (full)
    {
        std::cout << "The warehouse is full, insertQueue is waiting..." << "thread id = " << std::this_thread::get_id() <<std::endl;
    }
    return !full;
    });  
    enitem(task);         // 仓库放入产品
    repository_notEmpty.notify_all();  // 通知消费者仓库不为空
    enlock.unlock();  // 释放锁
};

void BlockingQueue::enitem(task_t task){
    std::unique_lock<std::mutex> lock(queueMtx);
    m_queue.push_back(std::move(task));
    m_num++;
    //enId++;
    lock.unlock();
}

task_t BlockingQueue::dequeue(){
    std::unique_lock<std::mutex> delock(deCounterMtx);  
    // 等待信号不为空的通知，wait 第二参数为true时 向下执行，否则一直等待
    repository_notEmpty.wait(delock, [this] {
        bool empty = m_queue.empty();
        if (empty)
        {
            std::cout << "The warehouse is empty, dequeue is waiting..." << "thread id = " << std::this_thread::get_id() <<std::endl;
        }
        return !empty;
    });
    task_t task=deitem();
    repository_notFull.notify_all();
    delock.unlock();
    return task;
    
};

task_t BlockingQueue::deitem(){
    std::unique_lock<std::mutex> lock(queueMtx);
    task_t task = m_queue.front();
    m_queue.pop_front();
    m_num--;
    //deId++;
    lock.unlock();
    return task;
}

