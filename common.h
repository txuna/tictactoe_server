#ifndef __COMMON_H_
#define __COMMON_H_

#include <errno.h>
#include "./include/nlohmann/json.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

typedef uint32_t uuid_t;
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

#define NONE_STONE 0
#define HOST_STONE 1
#define OTHER_STONE 2
#define DRAW_STONE -1

#define WIN_POINT 50
#define LOSE_POINT 15
#define DRAW_POINT 5

#define DRAW_GAME 0

enum PlayerState
{
    Lobby = 0,
    Playing = 1,
};

enum RoomState
{
    RoomReady = 0,
    RoomPlaying,
};

enum ServerMsg
{
    LoginResponse = 0, 
    RegisterResponse = 1, 
    LogoutResponse = 2,
    RoomCreateResponse = 3, 
    RoomStartResponse = 4,
    RoomExitResponse = 5,
    RoomJoinResponse = 6,
    RoomLoadResponse = 7,
    PlayerLoadResponse = 8,
    PlayerJoinRoomResponse = 9,
    PlayerExitRoomResponse = 10,
    WhoIsTurnResponse = 11,
    PlayerTurnResponse = 12,
};

enum ClientMsg
{
    Login = 0, 
    Register = 1, 
    Logout = 2,
    RoomCreate = 3, 
    RoomStart = 4,
    RoomExit = 5,
    RoomJoin = 6,
    RoomLoad = 7,
    PlayerLoad = 8,
    PlayerTurn = 9,
};

enum ErrorCode
{
    /* 0 ~ 100 Common */
    None = 0,
    MysqlError = 1,
    MysqlConnectionClose = 2,
    RedisError = 3,
    InvalidRequest = 4,
    InvalidToken = 5,

    /* 101 ~ 200 Account */
    NoneExistAccount = 101,
    InvalidPassword = 102,
    AlreadyExistEmail = 103,
    NoneExistAccountInRedis = 104,
    AlreadyLogined = 105,

    /* 201 ~ 300 Game */
    DuplicatedRoomTitle = 201,
    NoneExistRoom = 202,
    NoneExistPlayer = 203,
    AlreadyPlaying = 204,
    RoomIsFull = 205,
    AlreadyHost = 206,
    CannotExitRoomWithPlaying = 207,
    IsNotHost = 208,
    PlayerIsNotInRoom = 209,
    RoomIsNotStart = 210,
    NoneExistHostPlayer = 211,
    NoneExistOtherPlayer = 212,
    RoomIsNotFull = 213,
    IsNotYourTurn = 214,
    AlreadyRoomPlaying = 215,

    AlreadyLocatedStone = 216,
    CannotJoinThisRoomYourPoint = 217,
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

        json ProcessingMsg(int *err)
        {
            std::vector<byte_t> data(msg, msg+length);
            try
            {
                json j = json::from_msgpack(data); 
                *err = 0;
                return j;
            }
            catch(const std::exception& e)
            {
                *err = 1;
                return json();
            }
        }

};


#endif