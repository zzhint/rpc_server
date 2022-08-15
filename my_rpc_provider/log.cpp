#include "log.h"

bool block_queue::push(const std::string&item)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    if( full()){
        m_cond.notify_all();
        return false; 
    }
    m_queue.push(item);
    lk.unlock();
    m_cond.notify_all();
    return true;
}

bool block_queue::pop(std::string &item)
{
    std::unique_lock<std::mutex> lk(m_mutex);
    m_cond.wait(lk,[this]{return this->m_queue.size()>0;});
    item = m_queue.front();
    m_queue.pop();
    return true;

}

Log::Log()
{
    time_t now = time(nullptr);
    tm *nowtm = localtime(&now);
    char file_name[128];
    sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,
    nowtm->tm_mon+1,nowtm->tm_mday);
    file.open(file_name);
    std::thread t1(flush_log_thread);
    t1.detach();
}

void Log::write(std::string msg,Loglevel level=INFO)
{
    time_t now = time(nullptr);
    tm *nowtm = localtime(&now);
    char time_buf[128] = {0};
    sprintf(time_buf,"%d-%d-%d-%d-%d-%d =>[%s] ",
    nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday,
    nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,
    (level == INFO ? "INFO":"ERROR"));
    std::string new_msg = time_buf + msg; 
    m_queue.push(new_msg);
}

void Log::async_write_log()
{
    std::string single_log;
    while (m_queue.pop(single_log)){
        file<<single_log<<std::endl;
    }
}

void LOG_INFO(std::string msg,...){
    char buf[msg.size()+100];
    va_list arg_list;
    va_start(arg_list, msg);
    vsprintf(buf,msg.c_str(),arg_list);
    va_end(arg_list);
    Log::get_instance()->write(buf,INFO);
}

void LOG_ERROR(std::string msg,...){
    char buf[msg.size()+100];
    va_list arg_list;
    va_start(arg_list, msg);
    vsprintf(buf,msg.c_str(),arg_list);
    va_end(arg_list);
    Log::get_instance()->write(buf,ERROR);
}