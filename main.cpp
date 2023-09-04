#include <iostream>
#include "tevents.h"
#include "service.h"
#include "tmysql.h"
#include "main.h"
#include "model.h"
#include "controller.h"
#include "game.h"

int main(int argc, char **argv)
{
    std::string user, password, mport, host, sport, db_name;
    Game::GameObject *game_object = nullptr;
    Net::TcpSocket *socket = nullptr;

    if(ParseConfig(&user, &password, &mport, &host, &sport, &db_name) == C_ERR)
    {
        std::cout<<"Failed Parse Config File"<<std::endl;
        return 1;
    }

    Mysql::DB db = Mysql::DB(host, mport, user, password);

    if(db.Connect(db_name) == false)
    {
        std::cout<<"Failed Connect DB"<<std::endl;
        return 1;
    }

    socket = SetupServer(stoi(sport));

    if(socket == nullptr)
    {   
        std::cout<<"Failed Setup Socket Server"<<std::endl;
        delete socket;
        return 1;
    }

    //@@TODO GAME OBJECT 생성 
    game_object = new Game::GameObject(db); 
    if(game_object->GameLoop(socket) == C_ERR)
    {
        std::cout<<"Failed Start Game Loop"<<std::endl;
    }

    delete game_object;
    delete socket;
    return 0;
}


Net::TcpSocket *SetupServer(int port)
{
    Net::SockAddr *addr = new Net::SockAddr(port);
    Net::TcpSocket *socket = new Net::TcpSocket(addr, EPOLLIN | EPOLLOUT);

    if(socket->CreateSocket() == C_ERR)
    {
        perror("CreateSocket()");
        return nullptr;
    }

    if(socket->BindSocket() == C_ERR)
    {
        perror("BindSocket()");
        return nullptr;
    }

    if(socket->ListenSocket() == C_ERR)
    {
        perror("ListenSocket()");
        return nullptr;
    }

    return socket;
}


int ParseConfig(std::string *user, 
                std::string *password, 
                std::string *mport, 
                std::string *host, 
                std::string *sport,
                std::string *db_name)
{
    std::ifstream file("./config.conf");
    std::stringstream buffer;

    if ( file )
    {
        buffer << file.rdbuf();
        file.close();
    }

    std::istringstream is_file(buffer.str());
    std::string line; 
    while(std::getline(is_file, line))
    {
        std::istringstream is_line(line);
        std::string key; 
        if(std::getline(is_line, key, '='))
        {
            std::string value; 
            if(std::getline(is_line, value))
            {
                if(key.compare("server_port") == 0)
                {
                    *sport = value;
                }

                else if(key.compare("user") == 0)
                {
                    *user = value;
                }

                else if(key.compare("password") == 0)
                {
                    *password = value;
                }

                else if(key.compare("mysql_port") == 0)
                {
                    *mport = value;
                }

                else if(key.compare("host") == 0)
                {
                    *host = value;
                }

                else if(key.compare("database") == 0)
                {
                    *db_name = value;
                }
            }
        }
    }
   
    return C_OK;
}