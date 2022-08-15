#include "myrpcwebserver.h"
#include "myservice.h"

void WS::eventLoop(){
    epoll_event * events = m_epoll->get_events();
    while(true){
       
        int number = m_epoll->wait();
        for(int i=0;i<number;i++){
            int sockfd = events[i].data.fd;
            if(sockfd == m_listenfd){
                //处理新到的连接
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(m_listenfd,
                (struct sockaddr *)&client_address, 
                &client_addrlength);
                m_rpcs[connfd].init(connfd);
            }else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                m_epoll->removefd(sockfd);
                close(sockfd);
                m_rpcs->m_user_count--;
            }else if(events[i].events & EPOLLIN){
                //reactor 让工作线程自己读取数据
                //使用proactor因为比较简单
                m_rpcs[sockfd].read_once();
                m_tl->append_p(m_rpcs+sockfd);
                
            }else if(events[i].events & EPOLLOUT){
                bool b1 = m_rpcs[sockfd].write();
                if(!b1){
                    m_epoll->removefd(sockfd);
                    RPC::m_user_count--;
                }
                m_epoll->modfd(sockfd,EPOLLOUT);

            }
        }
        
    }
}
void WS::NotifyService(my_service *service){
    RPC::ServiceInfo service_info;
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    string service_name = pserviceDesc->name();
    int methodCnt = pserviceDesc->method_count();

    for (int i=0;i<methodCnt;++i){
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        string method_name = pmethodDesc->name();
        service_info.m_methodMap[method_name] = pmethodDesc;
    }
    service_info.m_service = service;
    m_serviceMap[service_name] = service_info;
    m_rpcs->m_serviceMap = &m_serviceMap;
    LOG_INFO("notify success");
}

void WS::Do(){
    ZkClient zkCli(m_zoo_server_ip,m_zoo_port);
    zkCli.Start();
    for (auto &sp : m_serviceMap){
        string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto &mp : sp.second.m_methodMap){
            string method_path = service_path + "/" + mp.first;
            string method_path_data = "127.0.0.1:" + std::to_string(m_server_port);
            zkCli.Create(method_path.c_str(),method_path_data.c_str(),
            method_path_data.size(),ZOO_EPHEMERAL);
        }
    }
    LOG_INFO("waiting...");
    eventLoop();
    
}

void WS::run(){
   NotifyService(ms);
   Do(); 
}


void WS::init(int port ,int zoo_port,string zoo_server, string user, 
    string passWord,
    string databaseName,int sql_num,
    int thread_num){
        m_sl = Sql_pool::getsql(); 
        m_sl->init("localhost", user, passWord, 
        databaseName, 3306, sql_num);
        
        m_rpcs->set_epoll(m_epoll);
        m_rpcs->set_root(m_root);
        
        m_tl = new threadpool<RPC>(thread_num);
        
        int listenfd = socket(PF_INET,SOCK_STREAM,0);
        assert(listenfd >= 0);
        struct linger tmp = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
        struct sockaddr_in address;
        bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
        int flag = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
        int ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
        assert(ret >= 0);
        
        ret = listen(listenfd, 5);//内核监听队列的最大长度
        assert(ret >= 0);
        m_epoll->addfd(listenfd,false,false);//当然不能oneshot
        ret = socketpair(PF_UNIX,SOCK_STREAM,0,m_pipefd);
        assert(ret!=-1);
        
        m_epoll->setnonblocking(m_pipefd[1]);
        m_epoll->addfd(m_pipefd[0],false);
        signal(SIGPIPE, SIG_IGN);
        m_listenfd = listenfd;
        //如果close了，不要关掉进程
        m_zoo_port = zoo_port;
        m_zoo_server_ip = zoo_server;
        m_server_port = port;

    }
WS::WS(){
    ms = new my_service();
    m_rpcs = new RPC[MAX_FD];
    char tmp[200];
    getcwd(tmp,200);
    m_root = tmp;
    m_root += "/root";
    m_epoll = new Myepoll;
}
WS::~WS(){
    delete [] m_rpcs;
    delete m_epoll;
    delete m_tl;
    
    delete ms;
};


std::unordered_map<std::string, RPC::ServiceInfo> WS::m_serviceMap = std::unordered_map<std::string, RPC::ServiceInfo>();