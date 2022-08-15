#ifndef MYEPOLL_H
#define MYEPOLL_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <utility>
class Myepoll{//把所有epoll操作封装到一起
    private:
        int m_epollfd;
        epoll_event *events;
    public:
        Myepoll(){
            m_epollfd = epoll_create(5);
            events = new epoll_event[10000];
        }
        ~Myepoll(){
            delete [] events;
        }
        epoll_event *get_events(){
            return events;
        }
        int wait(){
            int number = epoll_wait(m_epollfd,events,10000,-1);
            return number;
        }
        int setnonblocking(int fd);
        void addfd( int fd, bool one_shot, bool et=true);
        void removefd(int fd);
        void modfd( int fd, int ev);
};
#endif