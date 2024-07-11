#ifndef CONNECTION_POOL
#define CONNECTION_POOL
#include <queue>
#include <mysql/mysql.h>
#include <string>
#include <stdio.h>
#include "../lock/locker.h"
class connection_pool
{
private:
    // 最大连接数目
    int m_conn_num;
    // 当前已用连接数目
    int m_cur_coon;
    // 当前可用连接数目
    int m_free_coon;
    // 互斥锁,同步m_cur_coon,m_free_coon,m_queue
    lock m_lock;
    // 信号量，用于控制可用连接数目
    sem m_sem;
    // 可用连接队列
    std::queue<MYSQL*> m_queue;

    //数据库信息
    std::string m_url;
    int m_port;
    std::string m_username;
    std::string m_password;
    std::string m_DB_name;
    // 初始化可用连接数目和已用连接数目
    connection_pool(){
        m_free_coon = 0;
        m_cur_coon = 0;
    }
    ~connection_pool();
public:
    // 单例模式
    static connection_pool *getInstance(){
        static connection_pool m_conn_pool;
        return &m_conn_pool;
    }

    // 初始化数据库连接池
    void initConnection(std::string url,int port,std::string username,std::string password,std::string DB_name,int max_coon_num){
        // 数据库信息
        m_url = url;
        m_port = port;
        m_username = username;
        m_password = password;
        m_DB_name = DB_name;

        for(int i=0;i<m_conn_num;i++){
            MYSQL *conn = nullptr;
            conn = mysql_init(conn);
            if(!conn){
                perror("mysql init error: %s \n",mysql_error(conn));
                exit(1);
            }
            coon = mysql_real_connect(coon,m_url.c_str(),m_username.c_str(),m_password.c_str(),m_DB_name.c_str(),m_port,nullptr,0);
            if(!conn){
                perror("mysql coonect error: %s \n",mysql_error(conn));
                exit(1);
            }
            m_queue.push(coon);
            m_free_coon++;
        }
        m_conn_num = m_free_coon;
        // 初始化信号量为可用连接数目
        m_sem = sem(m_free_coon);
    }

    // 获取连接
    MYSQL *getConnection(){
        MYSQL *coon = nullptr;
        if(0 == m_queue.size()){
            return nullptr;
        }
        m_sem.wait();
        m_lock.wait();
        coon = m_queue.front();
        m_queue.pop();
        m_free_coon--;
        m_cur_coon++;
        m_lock.post();
        return coon;
    }

    // 释放连接,放回连接池
    bool releaseConnection(MYSQL* coon){
        if(!coon){
            return false;
        }
        m_lock.wait();
        m_queue.push(coon);
        m_free_coon++;
        m_cur_coon--;
        m_lock.post();
        m_sem.post();
        return true;
    }

    // 销毁连接池
    void destorypool(){
        m_lock.wait();
        while(!m_queue.empty()){
            // 关闭并且释放数据库连接
            mysql_close(m_queue.front());
            m_queue.pop();
        }
        m_free_coon = 0;
        m_cur_coon = 0;
        m_lock.post();
    }
};

class connection_pool_RALL{
private:
    MYSQL *coonRALL;
    connection_pool *poolRALL;
public:
    connection_pool_RALL(MYSQL** coon,connection_pool *coonPool){
        *coon = coonPool->getConnection();
        coonRALL = coon;
        poolRALL = coonPool;
    };
    ~connection_pool_RALL(){
        poolRALL->releaseConnection(coonRALL);
    }
};
#endif