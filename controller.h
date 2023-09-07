#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <random>

#include <vector>

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

            json Login(const json &req, socket_t fd, Model::PlayerList &players);
            json Register(const json &req);
            json Logout(const json &req, Model::PlayerList &players, Model::RoomList &rooms);
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

            json CreateRoom(const json &req, Model::PlayerList &players, Model::RoomList &rooms); 
            json JoinRoom(const json &req, Model::PlayerList &players, Model::RoomList &rooms);
            json StartRoom(const json &req);
            json ExitRoom(const json &req);
            json LoadRoom(const json &req);
    };
}

#endif