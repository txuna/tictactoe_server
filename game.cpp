#include "game.h"

Game::GameObject::GameObject(Mysql::DB &dbc, Redis::DB &rc)
: db_connection(dbc), redis_conn(rc)
{
    controller = new Controllers::Controller(db_connection, redis_conn, players, rooms, res_queue);
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
    EPOLLET가 아닌 EPOLLIN이라서 덜 읽더라도 다시 알림을 받을 수 있음
    정상적인 패킷이 2개가 붙어서 오더라도 length만큼 읽고 
    다음 패킷이 있더라도 알림을 받아서 읽을 수 있음
*/
void Game::GameObject::ProcessClientInput(Net::TcpSocket *socket, int mask)
{
    bool del_flag = false;
    if(mask == EPOLLIN)
    {
        Protocol *p = socket->ReadSocket();
        if(p == nullptr)
        {
            std::cout<<"Failed Read Packet Client: "<<socket->socket_fd<<std::endl;
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
        std::cout<<"Get Disconnect Signal from client: "<<socket->socket_fd<<std::endl;
        del_flag = true;
    }

    /*
        is_start
        현재 게임중일 경우 해당 유저가 host인지 other인지 확인하고 반대편 사람에게 승리했음을 알려주기
        게임중에 나갔다면 어떻게? 
    */
    if(del_flag)
    {
        std::cout<<"Client disconnected from server: "<<socket->socket_fd<<std::endl;
        ExitPlayerInPlaying(socket);
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
            res = controller->LoadPlayerInfo(j);
            type = ServerMsg::PlayerLoadResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::PlayerTurn:
        {
            res = controller->PlayerTurn(j);
            type = ServerMsg::PlayerTurnResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::RankLoad:
        {
            type = ServerMsg::RankLoadResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::AdminFetchPlayerRequest:
        {
            res = controller->AdminFetchPlayers(j);
            type = ServerMsg::AdminFetchPlayerResponse;
            //std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::AdminFetchRoomRequest:
        {
            res = controller->AdminFetchRooms(j);
            type = ServerMsg::AdminFetchRoomResponse;
            //std::cout << std::setw(4) << res << '\n';
            break;
        }

        case ClientMsg::AdminFetchGameRequest:
        {
            res = controller->AdminFetchGame(j);
            type = ServerMsg::AdminFetchGameResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }
            
        default:
        {
            std::cout<<"Invalid Protocol"<<std::endl;
            return C_ERR;
        }
    }

    Model::Response response(socket->socket_fd, res, type);
    res_queue.push(response);
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
        std::cout<<"1"<<std::endl;
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
        
        Net::TcpSocket *socket = el.LoadSocket(res.fd);
        if(socket != nullptr)
        {
            if(socket->SendSocket(res.res, res.type) == C_ERR)
            {

            }
        }

        res_queue.pop();
    }

    return;
}

void Game::GameObject::ExitPlayerInPlaying(Net::TcpSocket *socket)
{
    Model::Room *room = nullptr;
    Model::Player *exit_player = players.LoadPlayerFromSocketFd(socket->socket_fd);
    Model::Player *other_player = nullptr;

    if(exit_player == nullptr)
    {
        return;   
    }

    if(exit_player->state != PlayerState::Playing)
    {
        return;
    }

    room = rooms.LoadRoomFromRoomId(exit_player->room_id);
    if(room == nullptr)
    {
        return;
    }

    if(room->state == RoomState::RoomPlaying)
    {
        /* 나간 플레이어가 호스트라면 */
        if(exit_player->user_id == room->host_id)
        {
            other_player = players.LoadPlayer(room->other_id);
        }
        else
        {
            other_player = players.LoadPlayer(room->host_id);
        }
    }

    // 게임이 진행중일 때 나갈 시 포인트 정산
    if(room->is_start == true && other_player != nullptr)
    {
        Model::DatabaseUser *exit_user = controller->LoadUser(exit_player->user_id);
        Model::DatabaseUser *other_user = controller->LoadUser(other_player->user_id);

        if(exit_user != nullptr && other_user != nullptr)
        {
            exit_user->lose += 1;
            if(exit_user->point - LOSE_POINT < 0)
            {
                exit_user->point = 0;
            }
            else
            {
                exit_user->point -= LOSE_POINT;
            }
            other_user->win += 1;
            other_user->point += WIN_POINT;

            controller->UpdatePlayer(exit_user);
            controller->UpdatePlayer(other_user);
        }

        delete exit_user; 
        delete other_user;
    }

    if(room->is_start == true)
    {
        room->is_start = false;
    }

    rooms.LogoutPlayerInRoom(exit_player->user_id, room->room_id);

    /* 참여하고 있는 다른플레이어에게 다른 플레이어가 나갔음을 알림 */
    if(other_player != nullptr)
    {
        bool is_host = room->host_id == other_player->user_id ? true : false;
        json j = {
            {"error", ErrorCode::None}, 
            {"user_id", exit_player->user_id},
            {"is_host", is_host}
        };

        res_queue.push(Model::Response(other_player->fd, j, ServerMsg::PlayerExitRoomResponse));
    }

    return;
}