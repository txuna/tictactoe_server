#ifndef __GAME_H
#define __GAME_H

#include "sock.h"
#include "common.h"
#include "tmysql.h"
#include "tevents.h"
#include "controller.h"

namespace Game
{
    class GameObject
    {
        private:
            bool is_running = false;
            Epoll::EventLoop el;
            Mysql::DB &db_connection;

        public:
            GameObject(Mysql::DB &dbc);
            ~GameObject();
            int GameLoop(Net::TcpSocket *socket);
            void ProcessEvent(int retval);
            int ProcessAccept(Net::TcpSocket *socket, int mask);
            void ProcessClientInput(Net::TcpSocket *socket, int mask);
            int ProcessClientProtocol(Protocol *p);
            void SendGameState();
    };
}

#endif