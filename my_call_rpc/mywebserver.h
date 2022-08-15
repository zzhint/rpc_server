//去掉定时器，log
#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H
#include <string>
#include "myhttp.h"
#include "mythreadpool.h"
#include <netinet/in.h>
#include <assert.h>
#include <signal.h>


using std::string;


class WS{
public:
    static WS* getws(){
        static WS w;
        return &w;
    }
    void init(int port , string user, 
    string passWord,
    string databaseName, int sql_num,
    int thread_num);
    int m_listenfd;
    void eventLoop();

    Http * m_https;
    string m_root;
    Myepoll *m_epoll;
    threadpool<Http> *m_tl;
    int m_pipefd[2];//管道socket

private:
    static const int MAX_FD=65536;
    WS();
    ~WS();
};





#endif