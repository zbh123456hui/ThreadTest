#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <unistd.h>
#include <atomic>
#include <vector>
#include "BlockingQueue.h"

int done=0;
static const int kItemsToProduce = 20;//定义生产者能够生产的最大产品个数

BlockingQueue bqueue;
std::mutex workingMtx;
std::atomic_int itemCounter=1;         //任务队列计数器
std::atomic_int deitemCounter=1;
std::atomic_int taskCounter[20]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

void do_task(void* arg){
    if(done<100){
        //std::lock_guard<std::mutex> lock(workingMtx);
        done++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout<<"Doing "<<*((int*)arg)<<" ^th task..."<<std::endl;
    }else{
        handler_pt do_task=nullptr;
    }   
}

void entask(){
    while(itemCounter< kItemsToProduce)
    {
        task_t *task =new task_t{do_task,&taskCounter[itemCounter]};
        bqueue.enqueue(*task);    // 生产产品
        delete task;
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::cout <<"Thread "<<std::this_thread::get_id()<< " insert the " << itemCounter << " ^th task..." <<std::endl;
            itemCounter++;
        }
    }
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "Insert Thread exit.... " <<std::endl; 
    }
}
void detask(){
    bool readyToExit = false;
    
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (deitemCounter < kItemsToProduce)
        {
            task_t task =bqueue.dequeue();    // 消费产品
            //task_t task=bqueue.dequeue();
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                std::cout<<"Thread "<<std::this_thread::get_id()<< "dequeue the"<<deitemCounter<<" ^th task..."<<std::endl;
                deitemCounter++;
                (*(task.func))(task.arg);
                //delete task;
            }
        }
        else
        {
            readyToExit = true;
        }
        if (readyToExit)
            break;
    }
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Dequeue Thread "<<std::this_thread::get_id()<<" exit...." <<std::endl;
    }
}


int main()
{
    std::thread producer1(entask);
    std::thread producer2(entask);
    std::thread consumer1(detask);
    std::thread consumer2(detask);
    std::thread consumer3(detask);
    std::thread consumer4(detask);

    producer1.join();
    producer2.join();
    consumer1.join();
    consumer2.join();
    consumer3.join();
    consumer4.join();

    system("pause");
    return 0;
}