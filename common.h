#ifndef __COMMON_H_
#define __COMMON_H_

#include <errno.h>
#include "./include/nlohmann/json.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

typedef uint32_t socket_t;
typedef uint32_t protocol_t; 
typedef uint32_t length_t;
typedef uint8_t byte_t;

using json = nlohmann::json;
using namespace nlohmann::literals;

#define SOCKET_BUFFER 4096
#define CLIENT 0
#define SERVER 1

#define NAME_LEN 32
#define TITLE_LEN 32

#define C_OK 0
#define C_ERR -1
#define C_YET -2

enum ServerMsg
{
    LoginResponse = 0, 
    RegisterResponse = 1, 
    LogoutResponse = 2,
};

enum ClientMsg
{
    Login = 0, 
    Register = 1, 
    Logout = 2,
};

enum ErrorCode
{
    /* 0 ~ 100 Common */
    None = 0,
    MysqlError = 1,
    MysqlConnectionClose = 2,

    /* 101 ~ 200 Account */
    NoneExistAccount = 101,

    /* 201 ~ 300 Game */
};

class Protocol
{
    public:
        protocol_t protocol; 
        length_t length;
        byte_t *msg = nullptr;

        Protocol(protocol_t p, length_t l, byte_t *m)
        : protocol(p), length(l), msg(m)
        {
            
        }

        ~Protocol()
        {
            delete []msg;
        }

        json ProcessingMsg()
        {
            std::vector<byte_t> data(msg, msg+length);
            json j = json::from_msgpack(data); 
            return j;
        }
};


#endif