#include "game.h"

Game::GameObject::GameObject(Mysql::DB &dbc, Redis::DB &rc)
: db_connection(dbc), redis_conn(rc)
{
    
}

Game::GameObject::~GameObject()
{

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
        //Debug();
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
    if(mask == EPOLLIN)
    {
        Protocol *p = socket->ReadSocket();
        if(p == nullptr)
        {
            DelPlayerFromSock(socket->socket_fd);
            el.DelEvent(socket);
            return;
        }

        if(ProcessClientProtocol(socket, p) == C_ERR)
        {
            DelPlayerFromSock(socket->socket_fd);
            el.DelEvent(socket);
            delete p;
            return;
        }

        delete p;
    }
    else
    {
        DelPlayerFromSock(socket->socket_fd);
        el.DelEvent(socket);
        return;
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
        return C_ERR;
    }

    json res;
    protocol_t type;

    if(VerifyMiddleware(p, j) == false)
    {
        std::cout<<"Invalid Token"<<std::endl;
        res["error"] = ErrorCode::InvalidToken;
        return C_OK;
    }

    switch (p->protocol)
    {
        case ClientMsg::Login:
        {
            Controller::Authentication controller(db_connection, redis_conn);
            res = controller.Login(j);

            if(res["error"] == ErrorCode::None)
            {
                AddPlayer(res["user_id"], socket->socket_fd);
            }

            type = ServerMsg::LoginResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }
        
        case ClientMsg::Register:
        {
            Controller::Authentication controller(db_connection, redis_conn);
            res = controller.Register(j);
            type = ServerMsg::RegisterResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }
        
        /*
            players 목록에서도 삭제
            방목록에서도 삭제
        */
        case ClientMsg::Logout:
        {
            Controller::Authentication controller(db_connection, redis_conn);
            res = controller.Logout(j);

            if(res["error"] == ErrorCode::None)
            {
                DelPlayer(j["user_id"]);
            }

            type = ServerMsg::LogoutResponse;
            std::cout << std::setw(4) << res << '\n';
            break;
        }
            
        default:
            return C_ERR;
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

    std::string token = j["token"];
    uuid_t user_id = j["user_id"];

    std::string str_json; 
    ErrorCode result; 

    std::tie(result, str_json) = redis_conn.LoadString(std::to_string(user_id) + "_user");
    if(result != ErrorCode::None)
    {
        return false;
    }

    json stored_j = json::parse(str_json);

    if(stored_j["token"] != j["token"])
    {
        return false;
    }

    return true;
}


// 중복시 최근껄로 대입
void Game::GameObject::AddPlayer(uuid_t user_id, socket_t fd)
{
    Model::Player *p = new Model::Player(user_id, fd);

    players.push_back(p);
}

/*
    추후 방정보나 게임정보등에서도 배제하기
*/
void Game::GameObject::DelPlayer(uuid_t user_id)
{
    auto it = std::find_if(players.begin(), players.end(), 
                        [user_id](Model::Player *p){
                            return p->user_id == user_id;
                        });

    if(it != players.end())
    {
        Model::Player *p = *it;
        players.erase(it);
        delete p;
    }
}

void Game::GameObject::DelPlayerFromSock(socket_t fd)
{
    auto it = std::find_if(players.begin(), players.end(), 
                        [fd](Model::Player *p){
                            return p->fd == fd;
                        });

    if(it != players.end())
    {
        Model::Player *p = *it;
        players.erase(it);
        delete p;
    }
}

Model::Player *Game::GameObject::LoadPlayer(uuid_t user_id)
{
    auto it = std::find_if(players.begin(), players.end(), 
                        [user_id](Model::Player *p){
                            return p->user_id == user_id;
                        });

    if(it != players.end())
    {
        Model::Player *p = *it;
        return p;
    }

    return nullptr;
}


void Game::GameObject::Debug()
{
    std::cout<<"----------------"<<std::endl;
    for(auto i: players)
    {
        std::cout<<i->user_id<<std::endl;
    }
}


void Game::GameObject::SendGameState()
{
    
}