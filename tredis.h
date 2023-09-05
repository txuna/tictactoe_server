#ifndef __TREDIS_H
#define __TREDIS_H

#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <tuple>

#include "common.h"

namespace Redis
{
    class DB
    {
        private:
            redisContext* conn = nullptr;
            std::string host;
            std::string port;

        public:
            DB(std::string h, std::string p);
            ~DB();
            bool Connect();

            ErrorCode StoreString(std::string key, std::string value);
            std::tuple<ErrorCode, std::string> LoadString(std::string key);
    };
}

// object to string serialze? 

#endif 