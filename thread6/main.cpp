#include "ThreadPool.h"
#include "BlockingQueue.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

void taskFunc(void* arg)
{
    int nNum = *(int*)arg;
    cout << "thread: " << std::this_thread::get_id() << ", number=" << nNum << endl;
    //std::this_thread::sleep_for(std::chrono::seconds(1));
}
 int main()
{
    // 设置线程池最小5个线程，最大10个线程
    ThreadPool pool(5, 10);
    int i;
    user_t user[10];
    // 往任务队列中添加100个任务
    for (i = 0; i < 10; ++i){
        for(int j=0;j<10;j++){
            int* pNum = new int(10*i+j);
            task_t task={taskFunc, (void*)pNum};
            user[i].task[j]=task;
            
        }
        pool.Add(user[i]);
        cout<<"the "<<i<<"^th task is inserted."<<endl; 
    }
    sleep(1);
    return 0;
}
