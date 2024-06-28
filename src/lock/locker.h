#ifndef LOCKER
#define LOCKER
#include<pthread.h>
#include<semaphore.h>

class lock
{
private:
    pthread_mutex_t m_lock;
public:
    lock(){
        pthread_mutex_init(&m_lock,nullptr);
    }
    ~lock(){
        pthread_mutex_destroy(&m_lock);
    }
    pthread_mutex_t* get(){
        return &m_lock;
    }
    bool wait(){
        return pthread_mutex_lock(&m_lock);
    }
    bool post(){
        return pthread_mutex_unlock(&m_lock);
    }
};

class sem
{
private:
    sem_t m_sem;
public:
    sem(int val){
        sem_init(&m_sem,0,val);
    }
    ~sem(){
        sem_destroy(&m_sem);
    }
    bool wait(){
        return sem_wait(&m_sem);
    }
    bool post(){
        return sem_post(&m_sem);
    }
};


class cond
{
private:
    pthread_cond_t m_cond;
public:
    cond(){
        pthread_cond_init(&m_cond,nullptr);
    }
    ~cond(){
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *mutex){
        return pthread_cond_wait(&m_cond,mutex);
    }
    bool timedwait(pthread_mutex_t * mutex,timespec t){
        return pthread_cond_timedwait(&m_cond,mutex,&t);
    }
    bool singal(){
        return pthread_cond_signal(&m_cond);
    }
    bool broadcast(){
        return pthread_cond_broadcast(&m_cond);
    }
};


#endif