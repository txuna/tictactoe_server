#ifndef __GAME_H
#define __GAME_H

#include "sock.h"
#include "common.h"
#include "tmysql.h"
#include "tevents.h"
#include "middleware.h"
#include "tredis.h"
#include "controller.h"

#include <vector>

namespace Game
{
    class GameObject
    {
        private:
            bool is_running = false;
            Epoll::EventLoop el;
            Mysql::DB &db_connection;
            Redis::DB &redis_conn;
            std::vector<Model::Player*> players;
            
        public:
            GameObject(Mysql::DB &dbc, Redis::DB &rc);
            ~GameObject();
            int GameLoop(Net::TcpSocket *socket);
            void ProcessEvent(int retval);
            int ProcessAccept(Net::TcpSocket *socket, int mask);
            void ProcessClientInput(Net::TcpSocket *socket, int mask);
            int ProcessClientProtocol(Net::TcpSocket *socket, Protocol *p);
            void SendGameState();
            bool VerifyMiddleware(Protocol *p, json& j);
            void AddPlayer(uuid_t user_id, socket_t fd);
            void DelPlayer(uuid_t user_id);
            void DelPlayerFromSock(socket_t fd);
            Model::Player *LoadPlayer(uuid_t user_id);
            void Debug();
    };
}

#endif