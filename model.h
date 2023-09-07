#ifndef __MODEL_H
#define __MODEL_H

#include <string>
#include <stdint.h>

#include "common.h"

namespace Model
{
    class DatabaseUser
    {
        public:
            uuid_t user_id; 
            int win; 
            int lose; 
            int draw; 
            int point;

            DatabaseUser(uuid_t ui, int w, int l, int d, int p);
            ~DatabaseUser();
    };

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
            std::string token;
            PlayerState state;
            uuid_t user_id; 
            socket_t fd;
            int room_id;
            std::string name;
            Player(uuid_t u, socket_t f, PlayerState s, std::string t, std::string n);
            ~Player();
    };

    class Room
    {
        public:
            bool is_start;
            uuid_t host_id; 
            uuid_t other_id;
            int state;
            std::string title;
            int room_id;

            Room(uuid_t h, RoomState rs, std::string rt, int ri);
            ~Room();
    };

    class PlayerList
    {
        public:
            std::vector<Player*> players;

            PlayerList();
            ~PlayerList();

            void AppendPlayer(uuid_t user_id, socket_t fd, PlayerState state, std::string token, std::string name);
            void DeletePlayerFromUserId(uuid_t user_id);
            void DeletePlayerFromSocketFd(socket_t fd);
            Player* LoadPlayer(uuid_t user_id);
            Player* LoadPlayerFromSocketFd(socket_t fd);
            void Print();
            json LoadAllPlayers();
    };

    class RoomList
    {
        public:
            int room_index = 1;
            std::vector<Room*> rooms;

            RoomList();
            ~RoomList();
            void AppendRoom(Room *room);
            void DeleteRoom(int room_id);
            Room* LoadRoomFromRoomId(int room_id);
            Room* LoadRoomFromTitle(std::string title);
            void LogoutPlayerInRoom(uuid_t user_id, int room_id);
            void Print();
            json LoadAllRooms();
    };
}

#endif 