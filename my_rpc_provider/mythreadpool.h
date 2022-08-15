#ifndef MYTHREADPOOL_H
#define MYTHREADPOOL_H
#include <list>
#include <vector>
#include <thread>
#include "myrpc.h"
#include <exception>
using std::vector;
using std::list;
template<typename T>
class threadpool{
public:
    typedef std::thread make_thread;
    threadpool(int thread_number = 8, int max_request = 10000);
    ~threadpool(){}
    bool append_p(T *request);
    static void worker(threadpool *tl){
        tl->run();
    }
    void run();

private:
    int m_thread_number;        //线程池中的线程数
    int m_max_requests;         //请求队列中允许的最大请求数
    vector<make_thread> m_threads;       //描述线程池的数组，其大小为m_thread_number
    list<T *> m_workqueue; //请求队列
    std::mutex m_queuelocker;       //保护请求队列的互斥锁
    sem m_queuestat;            //是否有任务需要处理
    int m_actor_model;          //模型切换

};

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) :m_thread_number(thread_number),
  m_max_requests(max_requests)
{
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();
    for(int i=0;i<m_thread_number;i++){
        make_thread th(worker,this);
        th.detach();
        
    }

}

template <typename T>
bool threadpool<T>::append_p(T *request)
{
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
void threadpool<T>::run()
{
    while (true)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request){
            continue;
        }
        request->process();
    }
}
#endif



