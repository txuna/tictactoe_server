#ifndef __EVENTS_H
#define __EVENTS_H

#include <sys/epoll.h>
#include <vector>
#include <algorithm>
#include "sock.h"
#include "common.h"

#define MAX_EVENT_SIZE 512

namespace Epoll
{
    struct ev_t
    {
        int fd; 
        int mask;
    };

    class EventLoop
    {
        private:
            int epfd;

        public:
            std::vector<Net::TcpSocket*> events;
            ev_t *fired = nullptr;

            EventLoop();
            ~EventLoop();
            int CreateEventLoop();
            int AddEvent(Net::TcpSocket *socket);
            int DelEvent(Net::TcpSocket *socket);
            Net::TcpSocket *LoadSocket(socket_t fd);
            int FetchEvent();
    };
}

#endif