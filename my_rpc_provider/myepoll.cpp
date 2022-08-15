
#include "myepoll.h"
int Myepoll::setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}


void Myepoll::addfd(int fd, bool one_shot, bool et)
{
    epoll_event event;
    event.data.fd = fd;
    if (et){
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    }
    else{
        event.events = EPOLLIN | EPOLLRDHUP;
    }
    

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void Myepoll::removefd(int fd)
{
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void Myepoll::modfd( int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event);
}