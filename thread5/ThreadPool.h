#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include "BlockingQueue.h"

using namespace std;

// 线程池类
class ThreadPool
{
public:
    ThreadPool(int min, int max);
    // 添加任务
    void Add(handler_pt f, void* arg);
    void Add(task_t task);
    // 忙线程个数
    int Busynum();
    // 存活线程个数
    int Alivenum();
    ~ThreadPool();

private:
    // 任务队列
    BlockingQueue bqueue;
    thread managerID;   //管理者线程ID
    vector<thread> threadIDs;   //线程数组
    int minNum;   //最小线程数
    int maxNum;   //最大线程数
    int busyNum;   //忙的线程数
    int liveNum;    //存活的线程数
    int exitNum;    //要销毁的线程数

    mutex mutexPool;    //整个线程池的锁
    bool shutdown;    //是否销毁线程池，销毁为1，不销毁为0
    static void manager(void* arg);   //管理者线程
    static void worker(void* arg);   //工作线程
};

#endif