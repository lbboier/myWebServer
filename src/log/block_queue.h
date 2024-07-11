#ifndef BLOCK_QUEUE
#define BLOCK_QUEUE
#include <memory>
#include <queue>
#include "../lock/locker.h"
template <typename T>
class block_queue{
private:
    std::queue<T> b_queue;
    cond b_cond;
    lock b_lock;
    int b_size;
    int b_max_size;

public:
    block_queue(int max_queue_size){
        b_max_size = max_queue_size;
    }
    ~block_queue(){
    }

    bool empty(){
        return 0 == get_size();
    }

    bool full(){
        return b_max_size == get_size();
    }

    bool get_front(T &val){
        b_lock.wait();
        if(0 == b_size){
            b_lock.post();
            return false;
        }
        val = b_queue.front();
        b_lock.post();
        return true;
    }

    bool get_back(T &val){
        b_lock.wait();
        if(0 == b_size){
            b_lock.post();
            return false;
        }
        val = b_queue.back();
        b_lock.post();
        return true;
    }

    int get_size(){
        b_lock.wait();
        b_size = b_queue.size();
        b_lock.post();
        return b_size;
    }

    bool push(T &val){
        if(b_size>b_max_size){
            return false;
        }
        b_lock.wait();
        while(b_size == b_max_size){
            b_lock.post();
            b_cond.broadcast();
            b_lock.wait();
        }
        b_size++;
        b_queue.push(val);
        b_lock.post();
        b_cond.singal();
        return true;
    }

    bool pop(T &val){
        b_lock.wait();
        while(0 == b_size){
            b_cond.wait(b_lock.get());
        }
        b_size--;
        b_queue.pop();
        b_lock.post();
        return true;
    }
};
#endif