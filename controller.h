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
#include <queue>

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
            std::queue<Model::Response> &res_queue;

        public:
            Controller(Mysql::DB &dbc, Redis::DB &rc, Model::PlayerList &ps, Model::RoomList &rs, std::queue<Model::Response> &rq);
            ~Controller();

            json Login(const json &req, socket_t fd);
            json Register(const json &req);
            json Logout(const json &req);
            json LoadPlayerInfo(const json &req);
            json CreateRoom(const json &req); 
            json JoinRoom(const json &req);
            json StartRoom(const json &req);
            json ExitRoom(const json &req);
            json LoadRoom(const json &req);
            json PlayerTurn(const json &req);


            int CalculateGame(const json &req, json &response, Model::Room *room);
            void UpdatePlayerScore(int win_type, Model::Room *room);
    };
}

#endif