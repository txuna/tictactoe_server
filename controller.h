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

namespace Controller
{
    class BaseController
    {

    };
    
    class Authentication : public BaseController
    {
        private:
            Mysql::DB &db_connection;
            Service::AccountService *account_service;
            Service::PlayerService *player_service;

        public:
            Authentication(Mysql::DB &dbc);
            ~Authentication();

            void Login(const json &req);
            void Register(const json &req);
            void Logout(const json &req);
    };
}

#endif