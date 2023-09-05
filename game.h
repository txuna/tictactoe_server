#ifndef __GAME_H
#define __GAME_H

#include "sock.h"
#include "common.h"
#include "tmysql.h"
#include "tevents.h"
#include "middleware.h"
#include "tredis.h"
#include "controller.h"

namespace Game
{
    class GameObject
    {
        private:
            bool is_running = false;
            Epoll::EventLoop el;
            Mysql::DB &db_connection;
            Redis::DB &redis_conn;
            Middleware::AuthMiddleware auth_middleware;

        public:
            GameObject(Mysql::DB &dbc, Redis::DB &rc);
            ~GameObject();
            int GameLoop(Net::TcpSocket *socket);
            void ProcessEvent(int retval);
            int ProcessAccept(Net::TcpSocket *socket, int mask);
            void ProcessClientInput(Net::TcpSocket *socket, int mask);
            int ProcessClientProtocol(Protocol *p);
            void SendGameState();
            bool VerifyMiddleware(Protocol *p);
    };
}

#endif