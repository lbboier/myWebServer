#include "timer.h"

void timer_list::adjust_timer(timer *val) {
  if (!val) {
    return;
  }
  if (val == tail || val->next && val->expire_time <= val->next->expire_time) {
    return;
  }
  remove_timer(val);
  insert_timer(val);
  return;
}

void timer_list::del_timer(timer *val) {
  if (!val) {
    return;
  }
  remove_timer(val);
  delete val;
  return;
}

void timer_list::remove_timer(timer *val) {
  if (!val) {
    return;
  }
  if (head == val && tail == val) {
    head = nullptr;
    tail = nullptr;
    return;
  } else if (val == head) {
    head = head->next;
    head->prev = nullptr;
    val->next = nullptr;
    return;
  } else if (val == tail) {
    tail = tail->prev;
    tail->next = nullptr;
    val->prev = nullptr;
    return;
  } else {
    val->prev->next = val->next;
    val->next->prev = val->prev;
    val->prev = nullptr;
    val->next = nullptr;
    return;
  }
}

void timer_list::insert_timer(timer *val) {
  if (!val) {
    return;
  }
  if (!head && !tail) {
    head = tail = val;
    return;
  } else if (val->expire_time <= head->expire_time) {
    val->next = head;
    head->prev = val;
    head = val;
    return;
  } else if (val->expire_time > tail->expire_time) {
    val->prev = tail;
    tail->next = val;
    tail = val;
    return;
  } else {
    timer *tmp = head->next;
    while (tmp) {
      if (val->expire_time <= tmp->expire_time) {
        tmp->prev->next = val;
        val->prev = tmp->prev;
        val->next = tmp;
        tmp->prev = val;
        return;
      }
    }
  }
}

void timer_list::tick() {
  if (!head) {
    return;
  }
  timer_t cur = time(nullptr);
  timer *tmp = head;
  while (tmp) {
    if (cur < tmp->expire_time) {
      return;
    } else {
      tmp->cb_func(tmp->m_data);
      del_timer(tmp);
    }
    tmp = tmp->next;
  }
}

void utils::init(int timeslot) { m_TIMSLOT = timeslot; }

void utils::setnonblocking(int fd) {
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}

void utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode) {
  epoll_event event;
  event.data.fd = fd;
  if (1 == TRIGMode) {
    event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
  } else {
    event.events = EPOLLIN | EPOLLRDHUP;
  }
  if (one_shot) {
    event.events |= EPOLLONESHOT;
  }
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  set_nonblocking(fd);
}

static void utils::sig_handler(int sig) {
  int save_errno = errno;
  int msg = sig;
  send(u_pipefd[1], (char *)&msg, 1, 0);
  errno = save_errno;
}

void utils::addsig(int sig, void(handler)(int), bool restart = true) {
  struct sigaction sa;
  memset(&sa, '\0', sizeof(sa));
  sa.sa_handler = handler;
  if (restart) sa.sa_flags |= SA_RESTART;
  sigfillset(&sa.sa_mask);
  assert(sigaction(sig, &sa, NULL) != -1);
}

void utils::timer_headler() {
  m_timer_list->tick();
  alarm(m_TIMSLOT);
}

void utils::show_error(int connfd, const char *info) {
  send(connfd, info, strlen(info), 0);
  close(connfd);
}

int *utils::u_pipefd = 0;
int utils::u_epollfd = 0;

void cb_func(client_data *m_user_data) {
  epoll_ctl(utils::u_epollfd, EPOLL_CTL_DEL, m_user_data->socketfd, 0);
  assert(m_user_data);
  close(m_user_data->socketfd);
  http::m_user_count--;
}