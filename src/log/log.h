#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <string>
#include <ctime>
#include <cstring>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include "block_queue.h"
#include "../lock/locker.h"
class log{
private:
    char m_log_dir[128];
    char m_log_name[128];
    // 缓冲区大小
    int m_buf_size;
    // 日志文件最大行数
    int m_split_line;
    // 日志行数
    long long m_count;
    // 日志按照天分文件，记录当前的年月日
    struct tm *m_tm;
    // 缓冲区
    char* m_buf;
    // log文件指针
    FILE *m_fp;
    // 是否异步写入
    bool m_is_async;
    // 阻塞队列，
    block_queue<std::string> *m_block_queue;
    // 是否关闭日志
    bool m_close_log;
    // 日志文件的互斥写
    lock m_lock;
    log();
    ~log(){
        if(!m_fp){
            fclose(m_fp);
        }
    }
    
public:
    static log *getInstance(){
        static log mlog;
        return &mlog;
    }

    static void* flush_log_thread(void *arg){
        log::getInstance()->async_write_log();
    }

    bool init(const char* filename, int block_size = 0, int buf_size = 8192,int split_line = 500000){
        if(block_size > 1){
            m_is_async = true;
            m_block_queue = new block_queue<std::string>(block_size);
            pthread_t pid;
            pthread_create(&pid,nullptr,flush_log_thread,nullptr);
        }
        else{
            m_is_async = false;
        }
        
        m_buf_size = buf_size;
        m_buf = new char[m_buf_size];
        memset(m_buf,'\0',m_buf_size);
        m_split_line = split_line;

        time_t t = time(nullptr);
        m_tm = localtime(&t);

        const char *p  = strrchr(filename,'/');
        char full_log_name[256] = {0};
        char tail[16] = {0};
        snprintf(full_log_name,16,"%d_%02d_%02d_",m_tm->tm_year+1900,m_tm->tm_mon+1,m_tm->tm_mday+1);
        if(!p){
            snprintf(full_log_name,255,"%s%s",tail,filename);
        }
        else{
            // 路径，包含最后一个'/'
            strncpy(m_log_dir,filename,p-filename+1);
            strcpy(m_log_name,p+1);
            snprintf(full_log_name,255,"%s%s",tail,m_log_name);
        }
        m_fp = fopen(full_log_name,"a");
        if(!m_fp){
            return false;
        }
        return true;
    }

    void write_log(int level,const char *format, ...){
        // 创建新的日志文件
        struct timeval now = {0,0};
        gettimeofday(&now,nullptr);
        time_t t = now.tv_sec;
        struct tm *now_tm = localtime(&t);
        bool m_tm_flag = false;
        // 年月日是否更新
        if(now_tm->tm_mday!=m_tm->tm_mday || now_tm->tm_mon!=m_tm->tm_mon || now_tm->tm_year!=m_tm->tm_year){
            *m_tm = *now_tm;
            m_tm_flag = true;
        }
        char s[16];
        switch (level)
        {
        case 0:
            strcpy(s,"[debug]:");
            break;
        case 1:
            strcpy(s,"[info]:");
            break;
        case 2:
            strcpy(s,"[warn]:");
            break;
        case 3:
            strcpy(s,"[error]:");
            break;
        default:
            strcpy(s,"[info]:");
            break;
        }

        m_lock.wait();
        m_count++;
        if(m_tm_flag || m_count % m_split_line ==0){
            char new_log_name[256]={0};
            fflush(m_fp);
            fclose(m_fp);
            char tail[16] = {0};
            snprintf(tail,16,"%d_%02d_%02d_",m_tm->tm_year+1900,m_tm->tm_mon+1,m_tm->tm_mday+1);
            if(m_tm_flag){
                m_count = 0;
                snprintf(new_log_name,256,"%s%s%s",m_log_dir,tail,m_log_name);
            }
            else if(m_count % m_split_line ==0){
                snprintf(new_log_name,256,"%s%s%s.%lld",m_log_dir,tail,m_log_name,m_count/m_split_line);
            }
            m_fp = fopen(new_log_name,"a");
            if(!m_fp){
                throw std::exception();
            }
        }
        va_list valst;
        // 将传入的format参数赋值给valst,用于格式化输出
        va_start(valst,format);
        std::string write_str;
        int n = snprintf(m_buf,m_buf_size,"%d-%02d-%02d %02d:%02d:%02d.%06ld %s",m_tm->tm_year,m_tm->tm_mon,m_tm->tm_mday,m_tm->tm_hour,m_tm->tm_min,m_tm->tm_sec,now.tv_usec,s);
        int m = snprintf(m_buf+n,m_buf_size-n,format,valst);
        m_buf[m+n] = '\n';
        m_buf[m+n+1] = '\0';
        write_str = m_buf;
        m_lock.post();
        // 异步，写入阻塞队列
        if(m_is_async){
            if(!m_block_queue->push(write_str)){
                throw std::exception();
            }
        }
        // 同步，直接写入日志文件
        else{
            m_lock.wait();
            fputs(m_buf,m_fp);
            m_lock.post();
        }
        va_end(valst);
    }

    void async_write_log(){
        std::string write_str;
        while(m_block_queue->pop(write_str)){
            m_lock.wait();
            fputs(write_str.c_str(),m_fp);
            m_lock.post();
        }
    }

    void flush(void){
        m_lock.wait();
        fflush(m_fp);
        m_lock.post();
    }
};

#define LOG_DEBUG(format,...) if(0 == m_close_log){log::getInstance()->write_log(0,format,##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_INFO(format,...)  if(0 == m_close_log){log::getInstance()->write_log(1,format,##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_WARN(format,...)  if(0 == m_close_log){log::getInstance()->write_log(2,format,##__VA_ARGS__); log::getInstance()->flush();}
#define LOG_ERROR(format,...) if(0 == m_close_log){log::getInstance()->write_log(3,format,##__VA_ARGS__); log::getInstance()->flush();}

#endif;