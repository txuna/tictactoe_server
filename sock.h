#ifndef __SOCK_H_
#define __SOCK_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "common.h"


namespace Net
{
    class SockAddr
    {
        public:
            struct sockaddr_in adr;
            SockAddr(int port); 
            SockAddr(struct sockaddr_in a);
            ~SockAddr();
    };


    class TcpSocket
    {

        public:
            int mask;
            socket_t socket_fd;
            SockAddr* sock_addr;
            int type;
            
            TcpSocket(SockAddr *adr, int mask, socket_t fd);
            TcpSocket(SockAddr *adr, int mask);
            TcpSocket();
            ~TcpSocket();
            void SetSockAddr(SockAddr *adr);
            int CreateSocket();
            int BindSocket();
            int ListenSocket();
            TcpSocket *AcceptSocket();
            Protocol *ReadSocket();
            int SendSocket();
            int ReadHeader(protocol_t *protocol, length_t *length);
            int ReadMsg(length_t length, byte_t *msg);
    };
}


#endif