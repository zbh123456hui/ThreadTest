#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

typedef struct thread_pool_t thread_pool_t;
typedef void(*handler_pt)(void *);//函数指针

thread_pool_t *thread_pool_create(int thread_count, int queue_size);//创建线程池

int thread_pool_destroy(thread_pool_t * pool);//销毁线程池

int thread_pool_post(thread_pool_t * pool, handler_pt func, void *arg);//抛出任务

int wait_all_done(thread_pool_t * pool);

#endif