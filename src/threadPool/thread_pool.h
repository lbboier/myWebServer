#ifndef THREAD_POOL
#define THREAD_POOL
#include "../lock/locker.h"
#include "../sqlConnPool/connection_pool.h"
template <typename T>
class thread_pool {
private:
  pthread_t *m_thread_pool;
  connection_pool *m_conn_pool;
  int m_thread_num;
  int m_request_num;
  lock m_locker;
  sem m_sem;
  std::queue<T> m_queue;

public:
  thread_pool(connection_pool *conn_pool, int thread_num = 10, int request_num = 1000):m_thread_num(thread_num),m_request_num(request_num),m_thread_num(nullptr),m_conn_pool(conn_pool) {
    if(thread_num <=0 || request_num <=0){
      throw std::exception();
    }
    m_thread_pool = new pthread_t[m_thread_num];
    if(!m_conn_pool){
      throw std::exception();
    }
    for (int i = 0; i < m_thread_num; i++) {
      if(pthread_create(&m_thread_pool[i], nullptr, (void *)workder,
                               this)!=0){
                                delete m_thread_pool[];
                                throw std::exception();
                               }
      if(pthread_detach(&m_thread_pool[i])!=0){
        delete m_thread_pool[];
        throw std::exception();
      }
    }
  }

  ~thread_pool();

  bool append(T *request){
    m_locker.wait();
    if(m_queue.size()>=max_request){
      m_locker.post();
      return false;
    }
    else{
      m_queue.push(request);
      m_sem.post();
      m_locker.post();
    }
    return true;
  }
  static void *workder(void *arg) { 
    thread_pool *pool = (thread_pool*)arg;
    pool->run();
    return pool;
  }

  void run() {
    while(true){
      m_sem.wait()
      m_locker.wait();
      if(m_queue.empty()){
        m_locker.post();
        continue;
      }
      T &request = m_queue.front();
      m_queue.pop();
      m_locker.post();
      if(!request){
        continue;
      }
      // 从连接池中获取一个数据库连接
      request->mysql = m_conn_pool->getConnection();
      // 调用模板类的处理请求
      request->process();
      // 放回连接池
      m_conn_pool->releaseConnection(request->mysql);
    }
  }
};

#endif