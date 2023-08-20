#include "threadpool.h"
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
    for (i = 0; i < 100; ++i)
    {
        int* pNum = new int(i);
        pool.Add(taskFunc, (void*)pNum);
    }
	for (; i < 200; ++i)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		int* pNum = new int(i);
		pool.Add(taskFunc, (void*)pNum);
	}
    pool.~ThreadPool();
    return 0;
}
