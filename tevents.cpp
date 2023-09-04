#include "tevents.h"

Epoll::EventLoop::EventLoop()
{
    epfd = -1;
}

Epoll::EventLoop::~EventLoop()
{

}

int Epoll::EventLoop::CreateEventLoop()
{
    if(epfd != -1)
    {
        return C_ERR; // already create event loop
    }

    epfd = epoll_create(MAX_EVENT_SIZE);
    if(epfd == -1)
    {
        return C_ERR;
    }

    return C_OK;
}

int Epoll::EventLoop::AddEvent(Net::TcpSocket *socket)
{
    struct epoll_event ee = {0};   
    int fd = socket->socket_fd;
    int mask = socket->mask;

    ee.events = 0;
    ee.events = mask; 
    ee.data.fd = fd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ee) == -1)
    {
        return C_ERR;
    }

    events.push_back(socket);
    return C_OK;
}

int Epoll::EventLoop::DelEvent(Net::TcpSocket *socket)
{
    int fd = socket->socket_fd;
    int mask = socket->mask;
    struct epoll_event ee = {0};
    ee.data.fd = fd;
    ee.events = mask;
    epoll_ctl(epfd, EPOLL_CTL_DEL, ee.data.fd, &ee);

    auto it = std::find_if(events.begin(), events.end(), 
                           [fd](Net::TcpSocket *socket) {
                                return socket->socket_fd == fd;
                           });
    if(it != events.end())
    {
        Net::TcpSocket *rmsocket = *it;
        events.erase(it);
        delete rmsocket;
    }
    else
    {
        return C_ERR;
    }
    return C_OK;
}

Net::TcpSocket *Epoll::EventLoop::LoadSocket(socket_t fd)
{
    auto it = std::find_if(events.begin(), events.end(), 
                           [fd](Net::TcpSocket *socket) {
                                return socket->socket_fd == fd;
                           });

    if(it != events.end())
    {
        Net::TcpSocket *socket = *it; 
        return socket;
    }
    
    return nullptr;
}

int Epoll::EventLoop::FetchEvent()
{
    struct epoll_event ees[MAX_EVENT_SIZE];
    struct timeval tvp;
    tvp.tv_sec = 0;
    tvp.tv_usec = 100;
    int retval = epoll_wait(epfd, ees, MAX_EVENT_SIZE, tvp.tv_usec);
    
    if(retval <= 0)
    {
        return retval;
    }

    fired = new struct ev_t[retval];

    for(int i=0; i<retval; i++)
    {
        struct epoll_event *e = (ees + i);
        fired[i].fd = e->data.fd;
        fired[i].mask = e->events; 
    }

    return retval;
}
