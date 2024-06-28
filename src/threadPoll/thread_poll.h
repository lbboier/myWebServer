#ifndef THREAD_POLL
#define THREAD_POLL
#include "../lock/locker.h"
#include <queue>
#include <stdio.h>
template <typename T>
class thread_poll{
public:
    thread_poll(int thread_num = 10, int max_request = 1000){
        max_thread_num = thread_num;
        m_thread_poll = new pthread_t[max_thread_num];
        for(int i=0;i<max_thread_num;i++){
            int ret = pthread_create(&m_thread_poll[i],nullptr,(void *)workder,void * arg);
            if(0 == ret){
                pthread_detach(&m_thread_poll[i]);        
            }
        }
    }
    ~thread_poll();
private:
    pthread_t *m_thread_poll;
    int max_thread_num;
    int max_request;
    std::queue<T> t_queue;

public:
    static void *workder(){
        run();
    }
    void run(){
    }
};

#endif