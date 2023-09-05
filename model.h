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

    class Player
    {
        public:
            uuid_t user_id; 
            socket_t fd;
            Player(uuid_t u, socket_t f);
            ~Player();
    };
}

#endif 