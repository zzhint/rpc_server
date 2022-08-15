#include "myhttp.h"


const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";



string add_format(string sformat, ...)
{
    char buf[sformat.size()+100];
    va_list arg_list;
    va_start(arg_list, sformat);
    vsprintf(buf,sformat.c_str(),arg_list);
    va_end(arg_list);
    return buf;
}

unordered_map<string,string> m_files;


string read_file(string filename)
{
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

vector<string> splitstr(const string &s,const string p)
{
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

bool Http::read_once()
{
    char read_buf[m_buf_size];
    //只使用ET模式，一次性读完
    int bytes_read = recv(m_sockfd,read_buf, m_buf_size,0);
    if(bytes_read == -1){
        if (errno == EAGAIN || errno == EWOULDBLOCK){
            ;
        }
        return false;
    }
    m_read_string = read_buf;
    lines = splitstr(m_read_string,"\r\n");
    return true;
}

void Http::init()
{
    m_read_file = "";
    m_read_string="";
    m_write_string = "";
    lines.clear();
    m_url="";
    m_method=GET;
    m_linger = false; //是否长期连接
    m_content_length=0;
    m_host="";
    m_post_content="";
    m_cgi = false;
}

void Http::init(int sockfd)
{
    LOG_INFO("get a connection");
    m_sockfd = sockfd;
    m_epoll->addfd(m_sockfd,true);
    m_user_count++;

    init();
}

void Http::process_write(HTTP_CODE ret){
    switch (ret)
    {
    case BAD_REQUEST:
        
        m_write_string+=add_format("%s %d %s\r\n",
        "HTTP/1.1",404,error_404_title);
        m_write_string+=add_format(
            "Content-Length:%d\r\nConnection:%s\r\n\r\n"
            ,strlen(error_403_form),
            (m_linger == true) ? "keep-alive" : "close");
        m_write_string+=error_404_form;
        break;
    case FILE_REQUEST:
        m_write_string+=add_format("%s %d %s\r\n",
        "HTTP/1.1",200,ok_200_title);
        if(m_read_file.size()==0){
            m_read_file = "<html><body></body></html>";
        }
        m_write_string+=add_format(
            "Content-Length:%d\r\nConnection:%s\r\n\r\n"
            ,m_read_file.size(),
            (m_linger == true) ? "keep-alive" : "close");
        
    default:
        break;
    }
    m_write_string+=m_read_file;
}
bool Http::write(){
    send(m_sockfd,m_write_string.c_str(),m_write_string.size()+1,0);
    m_epoll->modfd(m_sockfd,EPOLLIN);

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
    

void Http::process(){
    LOG_INFO("do process");
    HTTP_CODE hc = process_read();
    process_write(hc);
    m_epoll->modfd(m_sockfd,EPOLLOUT);
}

HTTP_CODE Http::do_request(){
    string send_file_name=m_root;
    int pos = m_url.rfind("/");
    if(m_cgi && (m_url[pos+1]=='2' || m_url[pos+1]=='3')){
        int pos_yu = m_post_content.find("&");
        string user = m_post_content.substr(0,pos_yu);
        user = user.substr(5);
        string password = m_post_content.substr(pos_yu+1);
        password = password.substr(9);
        if(m_url[pos+1]=='3'){//注册
            fixbug::RegisterRequest request; 
            request.set_name(user);
            request.set_pwd(password);
            fixbug::RegisterResponse response; 
            fixbug::UserServiceRpc_Stub stub(new Myrpcchannel());
            LOG_INFO("do rpc: request");
            stub.Register(nullptr,&request,&response,nullptr);
            if(response.result().errcode() == 0){
                send_file_name +="/log.html";
            }else{
                LOG_ERROR(response.result().errmsg());
                send_file_name += "/registerError.html";
            }
            LOG_INFO("rpc success");

        }else if(m_url[pos+1] == '2'){
            fixbug::LoginRequest request; 
            request.set_name(user);
            request.set_pwd(password);
            fixbug::LoginResponse response; 
            fixbug::UserServiceRpc_Stub stub(new Myrpcchannel());
            LOG_INFO("do rpc: login");
            stub.Login(nullptr,&request,&response,nullptr);
            if(response.result().errcode() == 0){
                send_file_name +="/welcome.html";
            }else{
                LOG_ERROR(response.result().errmsg());
                send_file_name += "/logError.html";
            }
            LOG_INFO("rpc success");
        }
    }else if(m_url[pos+1] == '0'){
        send_file_name += "/register.html";
    }else if(m_url[pos+1] == '1'){
        send_file_name +="/log.html";
    }else if(m_url[pos+1] == '5'){
        send_file_name += "/picture.html";
    }else if(m_url[pos+1] == '6'){
        send_file_name += "/video.html";
    }else if(m_url[pos+1] == '7'){
        send_file_name +="/fans.html";
    }else{
        send_file_name += m_url;
    }
    m_read_file = read_file(send_file_name);
    ;
    return FILE_REQUEST;
}


HTTP_CODE Http::process_read(){
    CHECK_STATE state = CHECK_STATE_REQUESTLINE;
    for(int i=0;i<lines.size()+1;i++){
        string this_line;
        if(i<lines.size()){
            this_line = lines[i];
        }else{
            this_line = "";
        }
        
        
        switch(state){
            case CHECK_STATE_REQUESTLINE:{
                this_line+=" ";
                vector<string> vs1 = splitstr(this_line," ");
                if(vs1.size()!=3){
                    return BAD_REQUEST;
                }
                if(vs1[0] == "GET"){
                    m_method = GET;
                }else if(vs1[0] == "POST"){
                    m_method = POST;
                    m_cgi = true;
                }else{
                    return BAD_REQUEST;
                }
                m_url = vs1[1];
                if(m_url.substr(0,7) == "http://"){
                    m_url = m_url.substr(7);
                }
                if(m_url.substr(0,8) == "https://"){
                    m_url = m_url.substr(8);
                }
                if(m_url == "/"){
                    m_url += "judge.html";
                }
                if(vs1[2]!="HTTP/1.1"){
                    return BAD_REQUEST;
                }
                state = CHECK_STATE_HEADER;

                break;
            }
            case CHECK_STATE_HEADER:{
                if (this_line == ""){
                    if (m_method == GET){
                        //get 的信息已经收获完毕

                        return do_request();
                    }else{
                        state = CHECK_STATE_CONTENT;
                    }
                }else if(this_line.find("Connection:")!=string::npos){
                    if(this_line.find("keep-alive")!=string::npos){
                        m_linger = true;
                    }
                }else if(this_line.find("Content-Length:")!=string::npos){
                    m_content_length = stoi(this_line.substr(16));
                }else if(this_line.find("Host:")!=string::npos){
                    m_host = this_line.substr(6);
                }else{
                    ;//写入错误log
                }
                break;

            }
            case CHECK_STATE_CONTENT:{
                m_post_content = this_line.substr(0,m_content_length);
                return do_request();
            }
        }
        
    }
    return NO_REQUEST;
};
int Http::m_user_count = 0;
string Http::m_root = "";
Myepoll * Http::m_epoll = nullptr;
std::mutex Http::m_mutex;