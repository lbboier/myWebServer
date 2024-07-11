#ifndef TIMER_H
#define TIMER_H
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../http/http.h"
class timer;

// 客户端连接资源
struct client_data {
  sockaddr_in address;
  int socketfd;
  timer *m_timer;
};

class timer {
 public:
  timer() : prev(nullptr), next(nullptr) {}
  ~timer();
  time_t expire_time;
  timer *prev;
  timer *next;
  client_data *m_data;
  void (*cb_func)(client_data *);
};

class timer_list {
 public:
  timer_list() : head(nullptr), tail(nullptr) {}
  ~timer_list() {
    timer *tmp = head;
    while (tmp) {
      head = tmp->next;
      delete tmp;
      tmp = head;
    }
  }

 private:
  timer *head;
  timer *tail;

 public:
  // 从链表中调整
  void adjust_timer(timer *val);
  // 从链表中删除
  void del_timer(timer *val);
  // 从链表中移除
  void remove_timer(timer *val);
  // 插入到链表中
  void insert_timer(timer *val);
  // 定时任务处理过期连接
  void tick();
};

class utils {
 public:
  utils(){};
  ~utils(){};
  void init(int timeslot);
  int setnonblocking(int fd);
  void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);
  static void sig_handler(int sig);
  void addsig(int sig, void(handler)(int), bool restart = true);
  void timer_headler();
  void show_error(int coonfd, const char *info);

 private:
  static int *u_pipefd;
  timer_list *m_timer_list;
  static int u_epollfd;
  int m_TIMSLOT;
};

void cb_func(client_data *m_user_data);
#endif