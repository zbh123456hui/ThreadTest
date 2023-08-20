#include "ThreadPool.h"
#include "BlockingQueue.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

void taskFunc(void* arg)
{
    int nNum = *(int*)arg;
    cout << "thread: " << std::this_thread::get_id() << ", number=" << nNum << endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
 int main()
{
    // 设置线程池最小5个线程，最大10个线程
    ThreadPool pool(5, 10);
    int i;
    // 往任务队列中添加100个任务
    for (i = 1; i <= 100; ++i)
    {
        int* pNum = new int(i);
        pool.Add(taskFunc, (void*)pNum);
        cout<<"the "<<i<<"^th task is inserted."<<endl; 
    }
    sleep(1);
    return 0;
}
