//去掉定时器，log
#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H
#include <string>

#include "mythreadpool.h"
#include <netinet/in.h>
#include <assert.h>
#include <signal.h>
#include "mysqlpool.h"
#include "myrpc.h"
#include "myzookeeper.h"

using std::string;


class WS{
public:
    static WS* getws(){
        static WS w;
        return &w;
    }
    void init(int port ,int zoo_port,string zoo_server, string user, 
    string passWord,
    string databaseName, int sql_num,
    int thread_num);
    int m_listenfd;
    void eventLoop();
    void run();
    RPC * m_rpcs;
    string m_root;
    Myepoll *m_epoll;
    threadpool<RPC> *m_tl;
    Sql_pool *m_sl;
    int m_pipefd[2];//管道socket

    static std::unordered_map<std::string, RPC::ServiceInfo> m_serviceMap;
private:
    static const int MAX_FD=65536;
    my_service *ms;
    string m_zoo_server_ip;
    int m_zoo_port;
    int m_server_port;
    
    
    void NotifyService(my_service * service);
    void Do();
    WS();
    ~WS();
};

#endif