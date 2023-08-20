#include "ThreadPool.h"
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;
const int NUMBER = 2;

ThreadPool::ThreadPool(int min, int max)
{
    do
    {
        minNum = min;
        maxNum = max;
        busyNum = 0;
        liveNum = min;
        exitNum = 0;

        shutdown = false;
        // this:传递给线程入口函数的参数，即线程池
        managerID = thread(manager, this);

        threadIDs.resize(max);
        for (int i=0;i<min;++i){
            threadIDs[i]=thread(worker,this);
        }
        return;
    } while(0);
}

ThreadPool::~ThreadPool(){
    //阻塞回收管理者线程
    while(true){
        if(bqueue.size()==0){
            shutdown=true;
            if(managerID.joinable()) managerID.join();
            //唤醒阻塞的消费者线程
            for(int i=1;i<=maxNum;i++){
                if(threadIDs[i].joinable()) threadIDs[i].join();
            }
            return;
        }
    }
}

void ThreadPool::Add(user_t u){
    if (shutdown){
        return;
    }
    //添加任务
    bqueue.enqueue(u);
}

//void ThreadPool::Add(handler_pt f, void* a){
//    if (shutdown){
//        return;
//    }
//    //添加任务
//    bqueue.enqueue(task_t{f, a});
//}

int ThreadPool::Busynum(){
    mutexPool.lock();
    int busy = busyNum;
    mutexPool.unlock();
    return busy;
}

int ThreadPool::Alivenum(){
    mutexPool.lock();
    int alive = liveNum;
    mutexPool.unlock();
    return alive;
}

void ThreadPool::worker(void* arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    //工作者线程需要不停的获取线程池任务队列，所以使用while
    while (true){
        //判断线程池是否关闭了
        if (pool->shutdown){
            cout << "threadid: " << std::this_thread::get_id() << "exit......" << endl;
            return;
        }
        //取出task任务后，就可以在当前线程中执行该任务了
        cout << "thread: " << std::this_thread::get_id() << " start working..." << endl;
        //从任务队列中去除一个任务
        user_t user =pool->bqueue.dequeue();
        for(int i=0;i<10;i++){
            user.task[i].func(user.task[i].arg);
        }
        pool->busyNum++;
        // 任务执行完毕,忙线程解锁
        cout << "thread: " << std::this_thread::get_id() << " end working..." << endl;
        pool->busyNum--;
    }
}

// 检测是否需要添加线程还是销毁线程
void ThreadPool::manager(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    // 管理者线程也需要不停的监视线程池队列和工作者线程
    while (!pool->shutdown)
    {
        //每隔3秒检测一次
        std::this_thread::sleep_for(std::chrono::seconds(3));
        int queuesize = pool->bqueue.size();
        int livenum = pool->liveNum;
        int busynum = pool->busyNum;

        if (queuesize>livenum&&livenum<pool->maxNum)
        {
            int count = 0;
            for (int i = 0; i<pool->maxNum&&count<NUMBER&&pool->liveNum<pool->maxNum;++i)
            {
                if(pool->threadIDs[i].get_id()==thread::id())
                {
                    cout<<"Create a new thread..."<<endl;
                    pool->threadIDs[i]=thread(worker, pool);
                    count++;
                    pool->liveNum++;
                }
            }
        }
        if(busynum*2<livenum&&livenum>pool->minNum)
        {
            pool->exitNum = NUMBER;
        }
    }
}
