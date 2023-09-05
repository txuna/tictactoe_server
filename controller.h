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

            void Login(const json &req);
            void Register(const json &req);
            void Logout(const json &req);
            ErrorCode StoreUserInRedis(Model::Account *account, std::string token);
            void LoadUserFromRedis();
    };
}

#endif