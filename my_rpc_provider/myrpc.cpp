#include "myrpc.h"

unordered_map<string,string> m_files;
unordered_map<string,string> users;
string add_format(string sformat, ...)
{
    char buf[sformat.size()+100];
    va_list arg_list;
    va_start(arg_list, sformat);
    vsprintf(buf,sformat.c_str(),arg_list);
    va_end(arg_list);
    return buf;
}

string read_file(string filename){
    if (m_files.count(filename)){
        return m_files[filename];
    }else{
        std::fstream fm(filename);
        std::stringstream sm;
        sm<<fm.rdbuf();
        m_files[filename] = sm.str();
        return sm.str();
    }
    
}

vector<string> splitstr(const string &s,const string p){
	string::size_type pos = 0;
    string::size_type pre = 0;
	vector<string> ans;
    while((pos=s.find(p,pos)) !=string::npos){
        ans.push_back(s.substr(pre,pos-pre));
            pos = pos+p.size();
			pre = pos;
        }
	if(pre<s.size()){
        ans.push_back(s.substr(pre));
    }
	return ans;
}



bool RPC::read_once(){
        char read_buf[m_buf_size];
        //只使用ET模式，一次性读完
        int bytes_read = recv(m_sockfd,read_buf, m_buf_size,0);
        if(bytes_read == -1){
            if (errno == EAGAIN || errno == EWOULDBLOCK){
                ;
            }
            LOG_ERROR("read error");
            return false;
        }
        string tmp(read_buf,bytes_read);
        m_read_string = tmp;
        LOG_INFO("read it");
        return true;
    }

void RPC::init()
{
    m_read_file = "";
    m_read_string="";
    m_write_string = "";
    lines.clear();
    m_url="";

    m_linger = false; //是否长期连接
    m_content_length=0;
    m_host="";
    m_post_content="";
    m_cgi = false;
}

void RPC::init(int sockfd)
{
    LOG_INFO("get a rpc call");
    m_sockfd = sockfd;
    m_epoll->addfd(m_sockfd,true);
    m_user_count++;

    init();
}

bool RPC::write(){
    LOG_INFO("sending");
    send(m_sockfd,m_write_string.c_str(),m_write_string.size(),0);
    m_epoll->modfd(m_sockfd,EPOLLIN);
    LOG_INFO("send ok");
    if (m_linger)
    {
        init();
        return true;
    }
    else
    {
        return false;
    }
    
}

void RPC::process(){
    uint32_t header_size = 0;
    m_read_string.copy((char* )&header_size,4,0);

    string rpc_header_str = m_read_string.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    string service_name;
    string method_name; 
    uint32_t args_size; 

    if(rpcHeader.ParseFromString(rpc_header_str)){
        LOG_INFO("parse header ok");
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }else{
        LOG_ERROR("parse header error");
        return ;
    }
    string args_str = m_read_string.substr(4+header_size,args_size);
    


    auto it = m_serviceMap->find(service_name);
    if( it == m_serviceMap->end()){
        LOG_ERROR("service is not exist");
        return ;
    }
    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end()){
        LOG_ERROR("method is not exist");
        return ;
    }

    my_service *service = it->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second;
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    
    service->m_mysql = m_mysql;

    if(!request->ParseFromString(args_str)){
        LOG_ERROR("request parse error");
        return ;
    }else{
        LOG_INFO("request parse ok");
    }
    google::protobuf::Message * response =  service->GetResponsePrototype(method).New();
    LOG_INFO("calling method");
    service->CallMethod(method, nullptr, request, response, nullptr);  
    LOG_INFO("call method ok");
    if(!response->SerializeToString(&m_write_string)){
        LOG_ERROR("response serial error");
        return ;
    }else{
        LOG_INFO("response serial ok");
    }
    m_epoll->modfd(m_sockfd,EPOLLOUT);

};

int RPC::m_user_count = 0;

string RPC::m_root = "";
Myepoll * RPC::m_epoll = nullptr;
std::mutex RPC::m_mutex;
std::unordered_map<std::string, RPC::ServiceInfo> * RPC::m_serviceMap = nullptr;
