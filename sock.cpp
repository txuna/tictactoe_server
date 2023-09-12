#include "sock.h"
#include "tevents.h"
#include <iostream>


Net::SockAddr::SockAddr(int port)
{
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(port);
}

Net::SockAddr::SockAddr(struct sockaddr_in a)
{
    adr = a;
}

Net::SockAddr::~SockAddr()
{

}

Net::TcpSocket::TcpSocket(SockAddr *adr, int mask)
{
    socket_fd = -1;
    sock_addr = adr; 
    this->mask = mask;
}

Net::TcpSocket::TcpSocket(SockAddr *adr, int mask, socket_t fd)
{
    socket_fd = fd;
    sock_addr = adr;
    this->mask = mask;
}

Net::TcpSocket::TcpSocket()
{
    socket_fd == -1;
}

void Net::TcpSocket::SetSockAddr(SockAddr *addr)
{
    sock_addr = addr;
}

int Net::TcpSocket::CreateSocket()
{
    int flags;
    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1)
    {
        return C_ERR;
    }

    int on = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        return C_ERR;
    }

    if((flags = fcntl(socket_fd, F_GETFL, 0)) == -1)
    {
        return C_ERR;
    }
    
    if(fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        return C_ERR;
    }

    type = SERVER;

    return C_OK;
}

int Net::TcpSocket::BindSocket()
{
    if(bind(socket_fd, (struct sockaddr*)&sock_addr->adr, sizeof(sock_addr->adr)) == -1)
    {
        return C_ERR;
    }

    return C_OK;
}

int Net::TcpSocket::ListenSocket()
{
    if(listen(socket_fd, 5) == -1)
    {
        return C_ERR;
    }

    return C_OK;
}

Net::TcpSocket *Net::TcpSocket::AcceptSocket()
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    socket_t fd = accept4(socket_fd, (sockaddr*)&addr, &addr_len, SOCK_NONBLOCK);
    //socket_t fd = accept(socket_fd, (sockaddr*)&addr, &addr_len);

    if(fd <= 0)
    {
        return nullptr;
    }

    Net::SockAddr *sock = new Net::SockAddr(addr);
    Net::TcpSocket *socket = new Net::TcpSocket(sock, EPOLLIN | EPOLLHUP | EPOLLRDHUP | EPOLLERR, fd);
    socket->type = CLIENT;

    return socket;
}

Protocol *Net::TcpSocket::ReadSocket()
{
    protocol_t protocol; 
    length_t length; 
    byte_t *msg = nullptr;
    
    if(ReadHeader(&protocol, &length) == C_ERR)
    {
        return nullptr;
    }

    msg = new byte_t[length]();
    memset(msg, 0, length);
    
    if(ReadMsg(length, msg) == C_ERR)
    {
        return nullptr;
    }

    Protocol *p = new Protocol(protocol, length, msg);
    return p;
}

// Protocol이랑 length를 읽음
int Net::TcpSocket::ReadHeader(protocol_t *protocol, length_t *length)
{
    int ret = read(socket_fd, protocol, sizeof(protocol_t));
    if(ret < 0)
    {
        if(errno == EAGAIN)
        {
            return C_YET;
        }
        return C_ERR;
    }

    ret = read(socket_fd, length, sizeof(length_t));
    if(ret < 0)
    {
        return C_ERR;
    }

    return C_OK;
}

// length의 길이만큼 msg를 읽어들임
int Net::TcpSocket::ReadMsg(length_t length, byte_t *msg)
{
    int ret = read(socket_fd, msg, length);
    if(ret < 0)
    {
        return C_ERR;
    }

    return C_OK;
}

int Net::TcpSocket::SendSocket(json& j, protocol_t p)
{
    std::vector<byte_t> v = json::to_msgpack(j);
    byte_t *msg = reinterpret_cast<byte_t*>(v.data());
    length_t length = v.size();

    byte_t *buffer = new byte_t[sizeof(protocol_t) + sizeof(length_t) + length]();

    int offset = 0;
    memcpy(buffer+offset, &p, sizeof(protocol_t));
    offset += sizeof(protocol_t);
    memcpy(buffer+offset, &length, sizeof(length_t));
    offset += sizeof(length_t);
    memcpy(buffer+offset, msg, length);
    offset += length;

    int ret = write(socket_fd, buffer, offset);
    if(ret <= 0)
    {
        return C_ERR;
    }

    return C_OK;
}


Net::TcpSocket::~TcpSocket()
{
    if(socket_fd != -1)
    {
        close(socket_fd);
    }

    if(sock_addr != nullptr)
    {
        delete sock_addr;
    }
}