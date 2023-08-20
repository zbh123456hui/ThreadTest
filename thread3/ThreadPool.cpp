#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "ThreadPool.h"

typedef struct task_t{ //任务函数
    handler_pt func;
    void *arg;
}task_t;

typedef struct task_queue_t{//任务队列
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    task_t *queue;
}task_queue_t;

struct thread_pool_t{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_t *threads;
    task_queue_t task_queue;

    int closed;//检测线程池是否退出
    int started;//当前在执行的线程数

    int thread_count;  //线程池数
    int queue_size;    //任务队列数
};

static int
thread_pool_free(thread_pool_t * pool); 

static void *
thread_worker(void * arg){//入口函数
    thread_pool_t *pool=(thread_pool_t *)arg;
    task_queue_t *que;
    task_t task;
    for(;;){
        pthread_mutex_lock(&(pool->mutex));
        que=&pool->task_queue;
        //虚假唤醒
        //linux pthread_cond_signal
        //1.可能被信号唤醒
        //2.业务场景造成的
        while(que->count==0&&pool->closed==0){
            //先释放mutex
            //阻塞在condition（线程就休眠了）
            //===============================
            //解除阻塞
            //加上mutex
            pthread_cond_wait(&(pool->condition),&(pool->mutex));
        }
        //取任务执行任务
        if(pool->closed==1){
            break;
        }
        task=que->queue[que->head];
        que->head=(que->head+1)%pool->queue_size;
        que->count--;
        pthread_mutex_unlock(&(pool->mutex));
        (*(task.func))(task.arg);
    }
    pool->started--;
    pthread_mutex_unlock(&(pool->mutex));
    pthread_exit(NULL);
    return NULL;
}

thread_pool_t *
thread_pool_create(int thread_count, int queue_size){ //创建线程池
    thread_pool_t * pool;
    if(thread_count<=0||queue_size<=0){
        return NULL;
    }

    pool=(thread_pool_t *)malloc(sizeof(*pool));
    if(pool==NULL){
        return NULL;
    }

    pool->closed=pool->started=0;
    pool->thread_count=0; //当前线程数
    pool->queue_size=queue_size;

    pool->task_queue.head=pool->task_queue.tail=pool->task_queue.count=0;  //初始化任务队列
    pool->task_queue.queue=(task_t*)malloc(sizeof(task_t)*queue_size);  //按任务队列数分配内存
    if(pool->task_queue.queue==NULL){
        //free(pool);
        thread_pool_free(pool);
        return NULL;
    }

    pool->threads=(pthread_t *)malloc(sizeof(pthread_t)*thread_count); //按线程池数分配内存
    if(pool->threads==NULL){
        //free(pool->task_queue.queue);
        //free(pool);
        thread_pool_free(pool);
        return NULL;
    }

    int i=0;
    for(;i<thread_count;i++){    //生成线程
        if(pthread_create(&(pool->threads[i]),NULL,thread_worker,(void *)pool)!=0){
            thread_pool_free(pool);
            return NULL;
        }
        pool->started++;
        pool->thread_count++;
    }

    pthread_mutex_init(&pool->mutex,NULL);  //以动态方式创建互斥锁
    pthread_cond_init(&pool->condition,NULL); //初始化一个条件变量
    return pool;
}

static int
thread_pool_free(thread_pool_t * pool){  //释放线程池
    if(pool==NULL||pool->started<=0){
        return -1;
    }
    if(pool->threads){
        free(pool->threads);
        pool->threads=NULL;

        pthread_mutex_lock(&(pool->mutex));
        pthread_mutex_destroy(&(pool->mutex));
        pthread_cond_destroy(&(pool->condition));
    }

    if(pool->task_queue.queue){
        free(pool->task_queue.queue);
        pool->task_queue.queue=NULL;
    }
    free(pool);
    return 0;
}

int 
thread_pool_destroy(thread_pool_t * pool){  //销毁线程池
    if(pool==NULL){  //线程池不存在
        return -1;
    }

    if(pthread_mutex_lock(&(pool->mutex))!=0){  //互斥锁加锁出问题。在成功完成之后会返回零。
        return -2;                              //其他任何返回值都表示出现了错误。
    }

    if(pool->closed==1){  //线程池已退出
        return -3;
    }

    pool->closed=1;

    if( pthread_cond_broadcast(&(pool->condition))!=0|| //唤醒线程时出问题
        pthread_mutex_unlock(&(pool->mutex))!=0){    //互斥锁解锁出问题
        return -2;
    }

    wait_all_done(pool);
    thread_pool_free(pool);

    return 0;
}

int thread_pool_post(thread_pool_t * pool, handler_pt func, void *arg){ //抛出任务
    if(pool==NULL||func==NULL){
        return -1;
    }

    if(pthread_mutex_lock(&(pool->mutex))!=0){
        return -2;
    }

    if(pool->closed==1){
        pthread_mutex_unlock(&(pool->mutex));
        return -3;
    }

    if(pool->queue_size==pool->task_queue.count){
        pthread_mutex_unlock(&(pool->mutex));
        return -4;
    }
    task_queue_t *task_queue=&(pool->task_queue);
    task_t *task=&(task_queue->queue[task_queue->tail]);
    task->func=func;
    task->arg=arg;
    task_queue->tail=(task_queue->tail+1)%pool->queue_size;
    task_queue->count++;

    if( pthread_cond_signal(&(pool->condition))!=0){
        pthread_mutex_unlock(&(pool->mutex));
        return -5;
    }
    pthread_mutex_unlock(&(pool->mutex));
    return 0; 
}



int wait_all_done(thread_pool_t * pool){
    int i,ret=0;
    for(i=0;i<pool->thread_count;i++){
        if(pthread_join(pool->threads[i],NULL)!=0){
            ret=1;
        }
    }
    return ret;
}
