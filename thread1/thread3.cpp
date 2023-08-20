#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <unistd.h>

using namespace std;
typedef void(*handler_pt)(void *);//函数指针
typedef struct task_t{ //任务函数
    handler_pt func;
    void *arg;
}task_t;

int done=0;

void do_task(void* arg){
    if(done<100){
        done++;
        printf("doing %d task\n",done);
    }else{
        handler_pt do_task=nullptr;
    }   
}

static const int kItemsToProduce = 20;//定义生产者能够生产的最大产品个数

class BlockingQueue{
public:
    void insertQueue(){
        for (int i = 1;i <= kItemsToProduce;++i)
        {
            insertItem(m_queue, task);    // 生产产品
            {
                //std::lock_guard<std::mutex> lock(stdoutMutex);
                cout << "Insert the " << i << " ^th task..." << endl;
            }
        }
        {
            //std::lock_guard<std::mutex> lock(stdoutMutex);
            cout << "Insert Thread exit.... " << endl;
        }
    }
    void deQueue(int th_ID){
        bool readyToExit = false;
        while (true)
        {
            //this_thread::sleep_for(std::chrono::seconds(1));
            std::unique_lock<std::mutex> lock(itemCounterMtx);  // 仓库产品消费计数器保持多线程互斥
            if (itemCounter < kItemsToProduce)
            {
                task_t task = deItem(m_queue);    // 消费产品
                itemCounter++;  // 每消费一次进行计数器+1
                {
                    //std::lock_guard<std::mutex> lock(stdoutMutex);
                    cout << "Dequeue Thread " <<th_ID<<" doing the " <<itemCounter << "^th task..." << endl;
                    (*(task.func))(task.arg);
                }
            }
            else
            {
                readyToExit = true;
            }
            //lock.unlock();
            if (readyToExit)
                break;
        }
        {
            //std::lock_guard<std::mutex> lock(stdoutMutex);
            cout << "Dequeue Thread "<<th_ID<<" exit...." << endl;
        }
    }
    void changeSize(int new_Size){
        Max_Size=new_Size;
    }
    void addTask(handler_pt newtask,void* arg){
        task={newtask,NULL};
    }
private:
    void insertItem(deque<task_t>& itemQueue, task_t task){
        std::unique_lock<mutex> lock(queueMtx);
        repository_notFull.wait(lock, [&] {
        bool full = itemQueue.size() >= Max_Size;
        if (full)
        {
            //std::lock_guard<std::mutex> lock(stdoutMutex);
            cout << "The warehouse is full, insertQueue is waiting..." << "thread id = " << std::this_thread::get_id() << endl;
        }
        return !full;
    });
    itemQueue.push_back(task);         // 仓库放入产品
    repository_notEmpty.notify_all();  // 通知消费者仓库不为空
    lock.unlock();  // 释放锁
    }
    task_t deItem(deque<task_t>& itemQueue){
        task_t task;
        std::unique_lock<std::mutex> lock(queueMtx);

        // 等待信号不为空的通知，wait 第二参数为true时 向下执行，否则一直等待
        repository_notEmpty.wait(lock, [&] {
            bool empty = itemQueue.empty();
            if (empty)
            {
                //std::lock_guard<std::mutex> lock(stdoutMutex);
                cout << "The warehouse is empty, dequeue is waiting..." << "thread id = " << std::this_thread::get_id() << endl;
            }

            return !empty;
        });

        task = itemQueue.front();
        itemQueue.pop_front();
        repository_notFull.notify_all();
        lock.unlock();
        return task;
}
    
private:
    int Max_Size=10;           //队列最大容量
    deque<task_t> m_queue;   // 任务队列
    task_t task;               //设置任务
    int itemCounter=0;         //任务队列计数器
    //std::mutex stdoutMutex;   //多线程标准输出同步锁
    std::mutex queueMtx;       //互斥量，保护任务队列的输入输出
    std::mutex itemCounterMtx; //互斥量，保护任务队列的输出的线程安全
    std::condition_variable repository_notFull;     // 条件变量, 指产品仓库缓冲区不为满
    std::condition_variable repository_notEmpty;    // 条件变量, 指产品仓库缓冲区不为空
};



int main()
{
    BlockingQueue bqueue;
    bqueue.addTask(do_task,nullptr);
    std::thread producer(&BlockingQueue::insertQueue,&bqueue);
    std::thread consumer1(&BlockingQueue::deQueue,&bqueue,1);
    std::thread consumer2(&BlockingQueue::deQueue,&bqueue,2);
    std::thread consumer3(&BlockingQueue::deQueue,&bqueue,3);
    std::thread consumer4(&BlockingQueue::deQueue,&bqueue,4);

    producer.join();
    consumer1.join();
    consumer2.join();
    consumer3.join();
    consumer4.join();

    system("pause");
    return 0;
}