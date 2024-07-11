#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <cassert>
#include "../timer/timer.h"
const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;
const int TIMESLOT = 5;
class webServer{
public:
    webServer(){

    }
    ~webServer(){

    }
    void init_log();
    void init_http(){
        users = new http[MAX_FD];
    }
    void init_coon_pool();
    void init_timer();
    void init_thread_pool();
    void init_trig_mode();
    void timer_handler(){

    }
    void event_listen(){
        m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
        assert(m_listenfd >=0);
        if(0 == m_OPT_LINGER){
            struct linger tmp = {0, 1};
            setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
        }
        else if(1 == m_OPT_LINGER){
            struct linger tmp = {1, 1};
            setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
        }
        struct sockaddr_in address;
        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(m_port);
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        socklen_t socklen = sizeof(address);
        bind(m_listenfd, (sockaddr *)&address, socklen);
        assert(listen(m_listenfd, 10) == 0);
        m_epoolfd = epool_create(5);
        assert(m_epoolfd != -1);
        
    }
    void event_loop();
private:
    int m_port;
    std::string m_root;
    int m_log_write;
    int m_close_log;
    int m_actormodel;

    connection_pool *m_coon_pool;
    std::string m_user;
    std::string m_password;
    std::string m_DB_name;
    int m_sql_num;

    thread_pool<http> *m_thread_pool;
    int m_thread_num;

    http *users;
    int m_pipefd[2];
    int m_epoolfd;
    int m_socketfd;
    int m_listenfd;
    epool_event events[MAX_EVENT_NUMBER];
    
    int m_OPT_LINGER;
    int m_TRIGMode;
    int m_LISTENTrigmode;
    int m_COONTrigmode;

    client_data *m_user_data;

};

#endif