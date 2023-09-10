#include "game.h"

Game::GameObject::GameObject(Mysql::DB &dbc, Redis::DB &rc)
: db_connection(dbc), redis_conn(rc)
{
    controller = new Controllers::Controller(db_connection, redis_conn, players, rooms);
}

Game::GameObject::~GameObject()
{
    delete controller;

    // players 다지우기 

    // rooms 다 지우기 
}

int Game::GameObject::GameLoop(Net::TcpSocket *socket)
{
    is_running = true;

    if(el.CreateEventLoop() == C_ERR)
    {
        return C_ERR;
    }

    if(el.AddEvent(socket) == C_ERR)
    {
        return C_ERR;
    }

    while(is_running)
    {
        int retval = el.FetchEvent();
        ProcessEvent(retval);

        // @@TODO Send Game State All Player 
        SendGameState();
    }

    return C_OK;
}

void Game::GameObject::ProcessEvent(int retval)
{
    for(int i=0; i<retval; i++)
    {
        Epoll::ev_t e = el.fired[i];
        Net::TcpSocket *socket = el.LoadSocket(e.fd);

        switch (socket->type)
        {
        case SERVER:
            std::cout<<"CONNECTED NEW CLIENT"<<std::endl;
            if(ProcessAccept(socket, e.mask) == C_ERR)
            {
                continue;
            }
            break;
        
        case CLIENT:
            std::cout<<"PROCESSING CLIENT FD: "<<socket->socket_fd<<std::endl;
            ProcessClientInput(socket, e.mask);
            break;

        default:
            break;
        }
    }
}

int Game::GameObject::ProcessAccept(Net::TcpSocket *socket, int mask)
{
    if(mask == EPOLLIN)
    {
        Net::TcpSocket *c_socket = socket->AcceptSocket();
        if(c_socket != nullptr)
        {
            if(el.AddEvent(c_socket) == C_ERR)
            {
                perror("AddEvent()");
                return C_ERR;
            }
        }
    }

    return C_OK;
}

/*
    먼저 protocol부분이랑 length부분만 읽고 
    length부분에 따라서 그 다음 바이트를 읽는다. 

    읽었을 때 에러값 확인 
    만약 length가 SOCKET_BUFFER보다 크다면 
*/
void Game::GameObject::ProcessClientInput(Net::TcpSocket *socket, int mask)
{
    bool del_flag = false;
    if(mask == EPOLLIN)
    {
        Protocol *p = socket->ReadSocket();
        if(p == nullptr)
        {
            del_flag = true;
        }
        else
        {
            if(ProcessClientProtocol(socket, p) == C_ERR)
            {
                del_flag = true;
            }

            delete p;
        }
    }
    else
    {
        del_flag = true;
    }

    /*
        is_start
        현재 게임중일 경우 해당 유저가 host인지 other인지 확인하고 반대편 사람에게 승리했음을 알려주기
        아니면 여기서 포인트 정산?
    */
    if(del_flag)
    {
        Model::Player* player = players.LoadPlayerFromSocketFd(socket->socket_fd);
        if(player != nullptr)
        {
            if(player->state == PlayerState::Playing)
            {
                rooms.LogoutPlayerInRoom(player->user_id, player->room_id);
            }
        }
        players.DeletePlayerFromSocketFd(socket->socket_fd);
        el.DelEvent(socket);
    }

    return;
}

/*
    Client 입력 처리
*/
int Game::GameObject::ProcessClientProtocol(Net::TcpSocket* socket, Protocol *p)
{
    int err;
    json j = p->ProcessingMsg(&err);
    
    if(err == 1)
    {
        std::cout<<"Parsing Error"<<std::endl;
        return C_ERR;
    }

    json res;
    protocol_t type;

    if(VerifyMiddleware(p, j) == false)
    {
        std::cout<<"Invalid Token in Authentication Middleware"<<std::endl;
        res["error"] = ErrorCode::InvalidToken;
        return C_ERR;
    }

    switch (p->protocol)
    {
        case ClientMsg::Login:
        {
            res = controller->Login(j, socket->socket_fd);
            type = ServerMsg::LoginResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }
        
        case ClientMsg::Register:
        {
            res = controller->Register(j);
            type = ServerMsg::RegisterResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }
        
        case ClientMsg::Logout:
        {
            res = controller->Logout(j);
            type = ServerMsg::LogoutResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::RoomCreate:
        {
            res = controller->CreateRoom(j);
            type = ServerMsg::RoomCreateResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::RoomJoin:
        {
            res = controller->JoinRoom(j);
            type = ServerMsg::RoomJoinResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        /*
            상대방에게도 Response를 줘야함 
        */
        case ClientMsg::RoomStart:
        {
            res = controller->StartRoom(j);
            type = ServerMsg::RoomStartResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::RoomExit:
        {
            res = controller->ExitRoom(j);
            type = ServerMsg::RoomExitResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::RoomLoad:
        {
            res = controller->LoadRoom(j);
            type = ServerMsg::RoomLoadResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::PlayerLoad:
        {
            res = controller->LoadPlayer(j);
            type = ServerMsg::PlayerLoadResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }
            
        default:
        {
            std::cout<<"Invalid Protocol"<<std::endl;
            return C_ERR;
        }
    }

    if(socket->SendSocket(res, type) == C_ERR)
    {
        return C_ERR;
    }

    return C_OK;
}

/*
    Redis에 저장된 토큰 검사 
    p에 token이 포함되어 있는지 확인 필수
*/

bool Game::GameObject::VerifyMiddleware(Protocol *p, json& j)
{
    if(p->protocol == ClientMsg::Login 
        || p->protocol == ClientMsg::Register)
    {
        return true;
    }

    if(j.contains("token") == false
    || j.contains("user_id") == false)
    {
        return false;
    }

    if(j["token"].type() != json::value_t::string
    || j["user_id"].type() != json::value_t::number_unsigned)
    {
        return false;
    }

    std::string token = j["token"];
    uuid_t user_id = j["user_id"];

    Model::Player *player = players.LoadPlayer(user_id);
    if(player == nullptr)
    {
        return false;
    }

    if(player->token != token)
    {
        return false;
    }

    return true;
}

void Game::GameObject::Debug()
{
    std::cout<<"--------------------------------------"<<std::endl;
    players.Print();
    rooms.Print();
}

/*
    게임정보를 전송한다. 
    유저의 상태에 따라 다르게 전송 
    1. 게임중이라면 
    -> 방의 정보를 전송한다. 만약 0이라면 해당 방은 삭제됨을 알림

    2. 로비라면 
    -> 방정보를 전송한다. 
*/
void Game::GameObject::SendGameState()
{
    while(res_queue.empty() == false)
    {
        Model::Response res = res_queue.front();
        
                

        res_queue.pop();
    }
    return;
}