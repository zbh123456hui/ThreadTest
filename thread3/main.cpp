#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <future>

#include "ThreadPool.h"

int nums=0;
int done=0;

pthread_mutex_t lock;

void do_task(void* arg){
    usleep(10000);
    pthread_mutex_lock(&lock);
    if(done<100){
        done++;
        printf("doing %d task\n",done);
    }else{
        handler_pt do_task=nullptr;
    }   
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv){
    int threads=8;
    int queue_size=256;

/*
    if(argc==2){
        threads=atoi(argv[1]);
        if(threads<=0){
            printf("threads number error: %d\n",threads);
            return 1;
        }
    }else if(argc>2){
        threads=atoi(argv[1]);
        queue_size=atoi(argv[1]);
        if(threads<=0||queue_size<=0){
            printf("threads number or queue size error: %d,%d\n",threads,queue_size);
            return 1;
        }
    }
*/

    thread_pool_t *pool=thread_pool_create(threads,queue_size);
    if(pool==NULL){
        printf("thread pool create error!\n");
        return 1;
    }

    while(thread_pool_post(pool,&do_task,NULL)==0){
        pthread_mutex_lock(&lock);
        usleep(1000);
        nums++;
        pthread_mutex_unlock(&lock);
    }

    printf("add %d tasks\n",nums);

    wait_all_done(pool);

    printf("did %d tasks\n",done);
    thread_pool_destroy(pool);
    return 0;
}