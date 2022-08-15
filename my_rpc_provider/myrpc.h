#ifndef MYRPC_H
#define MYRPC_H
#include <sys/socket.h>
#include <errno.h>
#include <sys/epoll.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "myepoll.h"
#include <sys/mman.h>
#include <stdarg.h>
#include "mysqlpool.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

#include "user.pb.h"
#include "rpcheader.pb.h"
#include "log.h"
#include "myservice.h"


using std::string;
using std::vector;
using std::unordered_map;



string add_format(string sformat, ...);


string read_file(string filename);


struct Data{
    string user;
    string password;
    bool cgi=false;
    string url;
};
vector<string> splitstr(const string &s,const string p);

class RPC {//RPC, 包括读取，处理，返回
private:
    const static int m_buf_size = 2048; 
    string m_read_string;
    string m_write_string;
    string m_read_file;
    int m_sockfd=-1;
    vector<string> lines;
    string m_url;

    bool m_linger = false; //是否长期连接
    int m_content_length;
    string m_host;
    string m_post_content;
    bool m_cgi = false;
    

public:
    struct ServiceInfo{
        my_service *m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *>
        m_methodMap;
    };

    
public:
    static int m_user_count;
    static string m_root;
    static Myepoll *m_epoll;
    static std::mutex m_mutex;
    static std::unordered_map<std::string, ServiceInfo> *m_serviceMap;
    MYSQL *m_mysql;

public:
    bool read_once();
    void init();
    void init(int sockfd);
    void process(); 
    bool write();
    void static set_root(string root){
        m_root = root;
    }
    void static set_epoll(Myepoll *epoll){
        m_epoll = epoll;
    }
 

};



#endif