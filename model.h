#ifndef __MODEL_H
#define __MODEL_H

#include <string>
#include <stdint.h>
#include "sock.h"
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
            int permission;

            Account(uuid_t user_id, 
                    std::string email, 
                    std::string password, 
                    std::string salt, 
                    std::string name,
                    int permission);
                        
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
            int permission;
            Player(uuid_t u, socket_t f, PlayerState s, std::string t, std::string n, int permission);
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
            uuid_t who_is_turn;
            /* 0 : blank, 1 : host player, 2 : other player*/
            int board[9] = {NONE_STONE};
            int min_point; 
            int max_point;

            Room(uuid_t h, RoomState rs, std::string rt, int ri, int min_p, int max_p);
            ~Room();
            int CheckWin();
    };

    class PlayerList
    {
        public:
            std::vector<Player*> players;

            PlayerList();
            ~PlayerList();

            void AppendPlayer(uuid_t user_id, socket_t fd, PlayerState state, std::string token, std::string name, int permission);
            void DeletePlayerFromUserId(uuid_t user_id);
            void DeletePlayerFromSocketFd(socket_t fd);
            Player* LoadPlayer(uuid_t user_id);
            Player* LoadPlayerFromSocketFd(socket_t fd);
            void Print();
            json LoadAllPlayer();
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

    class Response
    {
        public:
            json res;
            socket_t fd;
            protocol_t type;

            Response(socket_t f, json r, protocol_t t);
    };
}

#endif 