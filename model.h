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
            PlayerState state;
            uuid_t user_id; 
            socket_t fd;
            Player(uuid_t u, socket_t f, PlayerState s);
            ~Player();
    };

    class Room
    {
        public:
            uuid_t host_user_id; 
            uuid_t other_user_id;
            int room_state;
            std::string room_title;
            int room_id;

            Room(uuid_t h, RoomState rs, std::string rt, int ri);
            ~Room();
    };
}

#endif 