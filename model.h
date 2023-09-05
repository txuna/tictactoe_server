#ifndef __MODEL_H
#define __MODEL_H

#include <string>
#include <stdint.h>

#include "common.h"

namespace Model
{
    class Account
    {
        public:
            uuid_t user_id; 
            std::string email;
            std::string password; 
            std::string salt;
            std::string name;

            Account(uuid_t user_id, 
                        std::string email, 
                        std::string password, 
                        std::string salt, 
                        std::string name);
                        
            ~Account();
    };

    class RedisUser
    {
        public:
            std::string name; 
            std::string token; 
            uuid_t user_id;

            RedisUser(std::string n, std::string token, uuid_t u);
            ~RedisUser();
    };
}

#endif 