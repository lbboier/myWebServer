#ifndef HTTP_H
#define HTTP_H
#include <in.h>
#include <string>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <map>
#include "../sqlConnPoll/connection_poll.h"
#include "../lock/locker.h"
#include "../log/log.h"
class http{

public:
    // 读取文件的名称长度大小
    static const int FILENAME_LEN = 2048;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    enum METHOD{
        GET=0,POST,HEAD,PUT,DELETE,TRACE,OPEIONE,CONNECT,PATH
    };
    enum CHECK_STATE{
        CHECK_REQUESTLINE=0,CHECK_STATE_HEADER,CHECK_CONTENT
    };
    enum HTTP_CODE{
        NO_REQUEST,GET_REQUEST,BAD_REQUEST,NO_RESOURCE,FORBIDDEN_REQUEST,FILE_REQUEST,INTERNAL_ERROR,CLOSE_CONNECTION
    };
    enum LINE_STATE{
        LINE_OK=0,LINE_BAD,LINE_OPEN
    };
    http();
    ~http();
    void init(int sockfd,const sockaddr_in &addr);
    void close_http(bool read_close=true);
    void process();
    // 读取浏览器端发来的全部数据
    bool read_once();
    // 写入响应报文
    bool write();
    sockaddr_in *get_address(){
        return &m_address;
    }
    // 同步线程初始化数据区读取表
    void init_mysql_result();
    // CGI使用线程池初始化数据库表
    void init_result_FILE(connection_poll *coon_poll);
private:
    void init();
    // 从m_read_buf读取请求报文
    HTTP_CODE process_read();
    // 向m_write_buf写入响应报文
    bool process_write();
    // 解析请求行
    HTTP_CODE parse_request_line(std::string &text);
    // 解析请求头
    HTTP_CODE parse_request_header(std::string &text);
    // 解析POST请求的content
    HTTP_CODE parse_request_content(std::string &text);
    // 生成响应报文
    HTTP_CODE do_request();
    // 指向未处理的报文
    char *getline(){
        return m_read_buf+m_start_line;
    }
    // 子状态机解析行
    LINE_STATE parse_line();
    void unmap();
    bool add_response(const char *format,...);
    bool add_content(const char *format,...);
    bool add_status_line(int status,const char *title);
    bool add_header(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_keep_alive();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    // 读为0，写为1
    int m_state;
private:
    int m_sockfd;
    sockaddr_in m_address;

    // 存储请求报文
    char m_read_buf[READ_BUFFER_SIZE];
    // 请求报文中最后一个字节的下一个位置
    int m_read_index;
    // 请求报文读取的位置
    int m_check_index;
    // 请求报文中已经解析的字符个数
    int m_start_line;

    // 存储响应报文
    char m_write_buf[WRITE_BUFFER_SIZE];
    // 响应报文写入的位置
    int m_write_index;

    int bytes_to_send;
    int bytes_havd_send;

    //主状态机的状态
    CHECK_STATE m_check_state;
    //子状态机的状态
    LINE_STATE m_line_state;
    // 请求方法
    METHOD m_method;

    // 请求报文
    // 文件根目录
    std::string doc_root;
    // 请求文件名称
    // char m_real_file[FILENAME_LEN];
    std::string m_real_file;
    // 请求资源的url
    std::string m_url;
    // http协议的版本
    std::string m_version;
    // 请求域名
    std::string m_host;
    // 请求主体长度
    int m_content_length;
    // 是否保持连接
    bool m_keep_alive;
    // 存储请求头
    std::string m_string;

    // 读取服务器上的文件地址
    std::string m_file_address;
    // 读取文件信息
    struct stat m_file_stat;
    // 
    struct iovec m_iv[2];
    // 是否启用POST
    int cgi;
    int m_TRIGMode;
    int m_close_log;

    std::string sql_user;
    std::string sql_password;
    std::string sql_name;

};

#endif