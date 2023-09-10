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

namespace Controllers
{

    class Controller
    {
        private:
            Mysql::DB &db_connection;
            Redis::DB &redis_conn;
            Service::AccountService *account_service;
            Service::PlayerService *player_service;
            Model::PlayerList &players; 
            Model::RoomList &rooms;

        public:
            Controller(Mysql::DB &dbc, Redis::DB &rc, Model::PlayerList &ps, Model::RoomList &rs);
            ~Controller();

            json Login(const json &req, socket_t fd);
            json Register(const json &req);
            json Logout(const json &req);
            json LoadPlayer(const json &req);
            json CreateRoom(const json &req); 
            json JoinRoom(const json &req);
            json StartRoom(const json &req);
            json ExitRoom(const json &InvalidRequest);
            json LoadRoom(const json &req);
    };
}

#endif