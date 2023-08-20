//非生产者消费者模型
#include <iostream>
#include <thread>
#include <mutex>
#include <list>
#include <unistd.h>
#include <pthread.h>

using namespace std;
typedef void(*handler_pt)(void *);//函数指针
int done=0;

typedef struct task_t{ //任务函数
    handler_pt func;
    void *arg;
}task_t;

void do_task(void* arg){
    usleep(10000);
    if(done<100){
        done++;
        printf("doing %d task\n",done);
    }else{
        handler_pt do_task=nullptr;
    }   
}

class A{
public:
    void inMsgRecvQueue(){
        for(int i=0;i<100;i++){
            cout<<"inMsgRecvQueue() executed, inserting an element"<<i<<endl;
            //my_mutex.lock();      //要操作共享数据，所以先加锁
            //std::lock_guard<std::mutex>sbguard(my_mutex);
            std::unique_lock<std::mutex>sbguard(my_mutex);
            task={&do_task,NULL};  //可封装为新的函数，用以分配任务
            msgRecvQueue.push_back(task);
            //my_mutex.unlock();    //共享数据操作完毕，解锁
        }
    }
    void outMsgRecvQueue(){
        for(int i=0;i<100;i++){
            bool result=outMsgLULProc();
            if(result==true){
                cout<<"OutMsgRecvQueue() executed, extracting an element from the container"<<endl;
            }else{
                cout<<"OutMsgRecvQueue() executed, but currently there is an empty element in the receiving message queue"<<i<<endl;
            }
            //if(result!=true){
            //    cout<<"OutMsgRecvQueue() executed, but currently there is an empty element in the receiving message queue"<<i<<endl;
            //}
        }
        cout<<"end"<<endl;
    }
    bool outMsgLULProc(){
        std::unique_lock<std::mutex> sbguard(my_mutex);  //sbguard是随意起的变量名
        //my_mutex.lock();
        if(!msgRecvQueue.empty()){
            task_t command=msgRecvQueue.front();
            (*(task.func))(task.arg);
            msgRecvQueue.pop_front();
            //my_mutex.unlock();
            return true;
        }
        //my_mutex.unlock();
        return false;
    }
    
private:
    std::list<task_t>msgRecvQueue;
    task_t task;
    std::mutex my_mutex;   //创建互斥量
};

int main(){
    A myobja;
    //std::thread myOutnMsgObj(&A::outMsgRecvQueue,&myobja);
    std::thread myInMsgObj(&A::inMsgRecvQueue,&myobja);  
     
    std::thread myOutnMsgObj(&A::outMsgRecvQueue,&myobja);
    myInMsgObj.join(); 
    myOutnMsgObj.join();
    cout<<"main() function is finished!"<<endl;
    return 0;
}