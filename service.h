#ifndef __SERVICE_H
#define __SERVICE_H

#include <tuple>

#include "tmysql.h"
#include "model.h"
#include "common.h"

namespace Service
{
    class AccountService
    {
        private:
            Mysql::DB &db_connection;

        public:
            AccountService(Mysql::DB &dbc);
            ~AccountService();
            std::tuple<ErrorCode, Model::Account*> LoadAccount(std::string req_email);
            std::tuple<ErrorCode, uint64_t> InsertAccount(std::string email, 
                                    std::string password, 
                                    std::string salt, 
                                    std::string name);

            ErrorCode DeleteUser(uint64_t user);
    };

    class PlayerService
    {
        private:
            Mysql::DB &db_connection;

        public:
            PlayerService(Mysql::DB &dbc);
            ~PlayerService();
            ErrorCode CreatePlayer(uint64_t user_id);
    };
}

#endif