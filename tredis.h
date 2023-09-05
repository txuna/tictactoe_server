#ifndef __TREDIS_H
#define __TREDIS_H

#include <hiredis/hiredis.h>
#include <iostream>
#include <string>

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
    };
}

#endif 