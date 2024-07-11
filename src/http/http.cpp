#include "http.h"



void removefd(int epollfd, int fd){
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
}

// 重置EPOLLONESHOT事件
void modfd(int epollfd, int fd, int ev, int TRIGMode){
    epoll_event event;
    event.data.fd = fd;
    if(1 == TRIGMode){
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    }
    else{
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    }
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void http::init(){

}

// 循环读取客户数据，直到无数据可读或者对方关闭连接
bool http::read_once(){
    if(m_read_index > READ_BUFFER_SIZE){
        return false;
    }
    int bytes_read;
    // LT Mode
    if(0 == m_TRIGMode){
        bytes_read = recv(m_epollfd,m_read_buf,READ_BUFFER_SIZE - m_read_index,0);
        if(bytes_read <= 0){
            return false;
        }
        m_read_index += bytes_read;
    }
    // ET Mode
    else{
        while(true){
            bytes_read = recv(m_epollfd,m_read_buf,READ_BUFFER_SIZE - m_read_index,0);
            if(-1 == bytes_read){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    break;
                }
                return false;
            }
            if(0 == bytes_read){
                return false;
            }
            m_read_index += bytes_read;
        }
    }
    return true;
    
}