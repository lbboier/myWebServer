#ifndef CONNECTION_POLL
#define CONNECTION_POLL
#include <queue>
#include <mysql/mysql.h>
#include <string>
#include "../lock/locker.h"
class connection_poll
{
private:
    int m_conn_num;
    sem m_sem;
    int m_cur_coon;
    int m_free_coon;
    std::string m_url;
    int m_port;
    std::string m_username;
    std::string m_password;
    std::string m_DB_name;
    std::queue<mysql> m_queue;
public:
    static connection_poll *getInstance(){
        static connection_poll m_conn_poll;
        return &m_conn_poll;
    }
    connection_poll(){
        m_conn_num = 0;
        m_cur_coon = 0;
    }
    ~connection_poll();
    mysql *getConnection(){

    }
    void initConnection(std::string url,int port,std::string username,std::string password,std::string DB_name){
        m_url = url;
        m_port = port;
        m_username = username;
        m_password = password;
        m_DB_name = DB_name;
        
    }
    bool releaseConnection(mysql& coon){

    }
    bool releaseAllConnection(){

    }
};

#endif