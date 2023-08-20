#include "BlockingQueue.h"
#include<deque>
#include <vector>

typedef struct User{
    int userId;
    std::queue<task_t> t_queue;
    bool canInsert;
}User;

typedef struct UserQueue{
    std::vector<User> u_queue;
}UserQueue;

UserQueue uqueue;

void enUser(User u){
    uqueue.u_queue.push_back(u);
}

task_t deTask(){   
    for(int i=0;i<uqueue.u_queue.size();i++){
        if(uqueue.u_queue[i].canInsert){
            task_t task=uqueue.u_queue[i].t_queue.front();
            uqueue.u_queue[i].t_queue.pop();
            return task;
        }
    }
    return;
}

void doTask(task_t task){
    test

}