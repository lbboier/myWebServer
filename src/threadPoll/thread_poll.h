#ifndef THREAD_POLL
#define THREAD_POLL
#include "../lock/locker.h"
#include "../sqlConnPoll/connection_poll.h"
template <typename T>
class thread_poll {
 public:
  thread_poll(connection_poll &conn_poll, int thread_num = 10, int request_num = 1000):m_thread_num(thread_num),m_request_num(request_num),m_thread_num(nullptr),m_conn_poll(conn_poll) {
    if(thread_num <=0 || request_num <=0){
      throw std::exception();
    }
    m_thread_poll = new pthread_t[m_thread_num];
    if(!m_conn_poll){
      throw std::exception();
    }
    for (int i = 0; i < m_thread_num; i++) {
      if(pthread_create(&m_thread_poll[i], nullptr, (void *)workder,
                               this)!=0){
                                delete m_thread_poll[];
                                throw std::exception();
                               }
      if(pthread_detach(&m_thread_poll[i])!=0){
        delete m_thread_poll[];
        throw std::exception();
      }
    }
  }
  ~thread_poll();

 private:
  pthread_t *m_thread_poll;
  connection_poll *m_conn_poll;
  int m_thread_num;
  int m_request_num;
  lock m_locker;
  sem m_sem;
  std::queue<T> m_queue;

 public:
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
    thread_poll *poll = (thread_poll*)arg;
    poll->run();
    return poll;
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
      // 从连接池中去一个数据库进行连接

      // 调用模板类的处理请求
      request->process();

      // 将数据库放回连接池
    }
  }
};

#endif