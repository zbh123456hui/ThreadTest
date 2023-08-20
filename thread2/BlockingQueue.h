#ifndef _BLOCKINGQUEUE_H
#define _BLOCKINGQUEUE_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <unistd.h>
#include <atomic>

typedef void(*handler_pt)(void *);//函数指针
typedef struct task_t{ //任务函数
    handler_pt func;
    void* arg;
}task_t;

class BlockingQueue{
public:
    BlockingQueue(int capacity):m_capacity(capacity){}
    BlockingQueue():BlockingQueue(10){}
    void enqueue(task_t task);
    task_t dequeue();
private:
    void enitem(task_t task);
    task_t deitem();

private:
    int m_capacity;           //队列最大容量
    std::deque<task_t> m_queue;   // 任务队列    
    std::mutex enCounterMtx;   //互斥量，保护任务队列的输入的线程安全
    std::mutex deCounterMtx;   //互斥量，保护任务队列的输出的线程安全
    std::mutex queueMtx;       //互斥量，保护任务队列的输入输出
    std::condition_variable repository_notFull;     // 条件变量, 指产品仓库缓冲区不为满
    std::condition_variable repository_notEmpty;    // 条件变量, 指产品仓库缓冲区不为空
    
//public:
    //std::atomic_int enId=-1;
    //std::atomic_int deId=0;
};

#endif