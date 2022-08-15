#ifndef MYHTTP_H
#define MYHTTP_H
#include <sys/socket.h>
#include <errno.h>
#include <sys/epoll.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "myrpcchannel.h"
#include "myepoll.h"
#include <sys/mman.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include "user.pb.h"
using std::string;
using std::vector;
using std::unordered_map;

string add_format(string sformat, ...);


string read_file(string filename);




enum HTTP_CODE
{
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
};//返回的状态，应该由read返回给http
enum CHECK_STATE
{
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
};//三部分，看看读取到哪里了
enum METHOD
{
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
};
struct Data{
    string user;
    string password;
    bool cgi=false;
    string url;
};
vector<string> splitstr(const string &s,const string p);

class Http{//http, 包括读取，处理，返回
private:
    const static int m_buf_size = 2048; 
    string m_read_string;
    string m_write_string;
    string m_read_file;
    int m_sockfd=-1;
    vector<string> lines;
    string m_url;
    METHOD m_method;
    bool m_linger = false; //是否长期连接
    int m_content_length;
    string m_host;
    string m_post_content;
    bool m_cgi = false;
public:
    static int m_user_count;
    static string m_root;
    static Myepoll *m_epoll;
    static std::mutex m_mutex;


public:
    bool read_once();
    void init();
    void init(int sockfd);

    HTTP_CODE process_read();
    HTTP_CODE do_request();
    void process_write(HTTP_CODE ret);
    void process(); //process 总处理，包括读和写。process_read读取理解用户发来信息，do_request
                    //来验证和准备好要发送的网页，process_write返回
    bool write();
    void static set_root(string root){
        m_root = root;
    }
    void static set_epoll(Myepoll *epoll){
        m_epoll = epoll;
    }

};





#endif