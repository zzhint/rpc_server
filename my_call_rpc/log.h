#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <queue>
#include <string>
#include <condition_variable>
#include <thread>
#include <fstream>
#include <iostream>
#include <time.h>
#include <stdarg.h>

class block_queue {
protected:
	std::queue<std::string> m_queue;
	
	std::mutex m_mutex;
	std::condition_variable m_cond;
    int m_max_dize;
 
public:
	block_queue(int max_size = 1000){
        if (max_size<=0){
            exit(-1);
        }
        m_max_dize = max_size;
    }
    bool full(){
        if(m_queue.size()>=m_max_dize){
            return true;
        }else{
            return false;
        }
    }

    bool push(const std::string&item);
    bool pop(std::string &item);
	
};

enum Loglevel{
    INFO,
    ERROR,
};

class Log{
public:
    static Log *get_instance(){
        static Log l;
        return &l;
    }
    static void flush_log_thread(){
        Log::get_instance()->async_write_log();
    }
    void write(std::string msg,Loglevel level);
private:
    block_queue m_queue;
    std::ofstream file;
    Log();
    virtual ~Log(){
        file.close();
    }
    void async_write_log();
};

void LOG_INFO(std::string msg,...);
void LOG_ERROR(std::string msg,...);

#endif