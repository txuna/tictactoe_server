#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <random>

#include "common.h"
#include "tmysql.h"
#include "service.h"
#include "tredis.h"

using json = nlohmann::json;
using namespace nlohmann::literals;

namespace Controller
{
    class BaseController
    {

    };
    
    class Authentication : public BaseController
    {
        private:
            Mysql::DB &db_connection;
            Redis::DB &redis_conn;
            Service::AccountService *account_service;
            Service::PlayerService *player_service;

        public:
            Authentication(Mysql::DB &dbc, Redis::DB &rc);
            ~Authentication();

            json Login(const json &req);
            json Register(const json &req);
            json Logout(const json &req);
            ErrorCode StoreUserInRedis(Model::Account *account, std::string token);
    };

    class RoomController : public BaseController
    {
        private:
            Mysql::DB &db_connection;
            Redis::DB &redis_conn;
            Service::PlayerService *player_service;

        public:
            RoomController(Mysql::DB &dbc, Redis::DB &rc);
            ~RoomController();

            std::tuple<json, Model::Room*> CreateRoom(const json &req, int *room_index); 
            std::tuple<json, uuid_t> JoinRoom(const json &req);
            json StartRoom(const json &req);
            json ExitRoom(const json &req);
            json LoadRoom(const json &req);
            ErrorCode StoreRoomInRedis(Model::Room *room, std::string key);
            ErrorCode ChangePlayer(uuid_t user_id, json player_j, PlayerState state);
            std::tuple<ErrorCode, json> LoadPlayer(uuid_t user_id);
    };
}

#endif