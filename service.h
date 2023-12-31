#ifndef __SERVICE_H
#define __SERVICE_H

#include <tuple>

#include "tmysql.h"
#include "model.h"
#include "common.h"
#include "tredis.h"

namespace Service
{
    class AccountService
    {
        private:
            Mysql::DB &db_connection;
            Redis::DB &redis_conn;

        public:
            AccountService(Mysql::DB &dbc, Redis::DB &rc);
            ~AccountService();
            std::tuple<ErrorCode, Model::Account*> LoadAccountFromUserId(uuid_t req_user_id);
            std::tuple<ErrorCode, Model::Account*> LoadAccount(std::string req_email);
            std::tuple<ErrorCode, uuid_t> InsertAccount(std::string email, 
                                    std::string password, 
                                    std::string salt, 
                                    std::string name);

            ErrorCode DeleteUser(uuid_t user);
    };

    class PlayerService
    {
        private:
            Mysql::DB &db_connection;
            Redis::DB &redis_conn;

        public:
            PlayerService(Mysql::DB &dbc, Redis::DB &rc);
            ~PlayerService();
            ErrorCode CreatePlayer(uuid_t user_id);
            ErrorCode UpdatePlayer(uuid_t user_id, Model::DatabaseUser *user);
            std::tuple<ErrorCode, Model::DatabaseUser*> LoadPlayer(uuid_t user_id);
    };
}

#endif