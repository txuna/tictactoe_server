#ifndef __GAME_H
#define __GAME_H

#include "sock.h"
#include "common.h"
#include "tmysql.h"
#include "tevents.h"
#include "middleware.h"
#include "tredis.h"
#include "controller.h"

#include <queue>
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
            Model::PlayerList players;
            Model::RoomList rooms;
            std::queue<Model::Response> res_queue;
            Controllers::Controller *controller = nullptr;
            Net::TcpSocket *admin_socket = nullptr;

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

            void ExitPlayerInPlaying(Net::TcpSocket *socket);
            void Debug();
    };
}

#endif