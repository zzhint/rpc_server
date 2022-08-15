#include "mywebserver.h"

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
                m_https[connfd].init(connfd);
            }else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                m_epoll->removefd(sockfd);
                close(sockfd);
                m_https->m_user_count--;
            }else if(events[i].events & EPOLLIN){
                //reactor 让工作线程自己读取数据
                //使用proactor因为比较简单
                m_https[sockfd].read_once();
                m_tl->append_p(m_https+sockfd);
                
            }else if(events[i].events & EPOLLOUT){
                bool b1 = m_https[sockfd].write();
                if(!b1){
                    m_epoll->removefd(sockfd);
                    Http::m_user_count--;
                }
                m_epoll->modfd(sockfd,EPOLLOUT);

            }
        }
        
    }
}


void WS::init(int port , string user, 
    string passWord,
    string databaseName,int sql_num,
    int thread_num){

        LOG_INFO("start webserver");

        Http::set_epoll(m_epoll);
        Http::set_root(m_root);

        m_tl = new threadpool<Http>(thread_num);
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
        if(ret>=0){
            LOG_INFO("bind ok");
        }else{
            LOG_ERROR("port can't be used");
        }

        ret = listen(listenfd, 5);//内核监听队列的最大长度
        if(ret>=0){
            LOG_INFO("listen ok");
        }else{
            LOG_ERROR("listen error");
        }

        m_epoll->addfd(listenfd,false,false);//当然不能oneshot
        ret = socketpair(PF_UNIX,SOCK_STREAM,0,m_pipefd);
        
        m_epoll->setnonblocking(m_pipefd[1]);
        m_epoll->addfd(m_pipefd[0],false);
        signal(SIGPIPE, SIG_IGN);
        m_listenfd = listenfd;
        //如果close了，不要关掉进程

    }
WS::WS(){

    m_https = new Http[MAX_FD];
    char tmp[200];
    getcwd(tmp,200);
    m_root = tmp;
    m_root += "/root";
    m_epoll = new Myepoll;
}
WS::~WS(){
    delete [] m_https;
    delete m_epoll;
    delete m_tl;

};