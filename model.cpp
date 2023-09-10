#include "model.h"

Model::DatabaseUser::DatabaseUser(uuid_t ui, int w, int l, int d, int p)
: user_id(ui), win(w), lose(l), draw(d), point(p)
{

}

Model::DatabaseUser::~DatabaseUser()
{

}


Model::Account::Account(uuid_t user_id, 
                        std::string email, 
                        std::string password, 
                        std::string salt, 
                        std::string name)
{
    this->user_id = user_id; 
    this->email = email; 
    this->password = password; 
    this->salt = salt; 
    this->name = name;
}

Model::Account::~Account()
{

}

Model::Player::Player(uuid_t u, socket_t f, PlayerState s, std::string t, std::string n)
: user_id(u), fd(f), state(s), token(t), room_id(0), name(n)
{

}

Model::Player::~Player()
{

}

Model::Room::Room(uuid_t h, RoomState rs, std::string rt, int ri)
: host_id(h), state(rs), title(rt), room_id(ri), other_id(0), is_start(false)
{
    
}

Model::Room::~Room()
{

}

Model::PlayerList::PlayerList()
{

}

Model::PlayerList::~PlayerList()
{

}

void Model::PlayerList::AppendPlayer(uuid_t user_id, 
                                     socket_t fd, 
                                     PlayerState state, 
                                     std::string token, 
                                     std::string name)
{
    Model::Player* p = LoadPlayer(user_id);
    /* 기존 유저라면 토큰만 갱신 */
    if(p != nullptr)
    {
        p->token = token;
        return;
    }

    Model::Player* player = new Model::Player(user_id, fd, state, token, name);
    players.push_back(player);
}

void Model::PlayerList::DeletePlayerFromUserId(uuid_t user_id)
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

void Model::PlayerList::DeletePlayerFromSocketFd(socket_t fd)
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

Model::Player *Model::PlayerList::LoadPlayerFromSocketFd(socket_t fd)
{
    auto it = std::find_if(players.begin(), players.end(), 
                        [fd](Model::Player *p){
                            return p->fd == fd;
                        });

    if(it != players.end())
    {
        Model::Player *p = *it;
        return p;
    }

    return nullptr;
}

Model::Player *Model::PlayerList::LoadPlayer(uuid_t user_id)
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

json Model::PlayerList::LoadAllPlayers()
{
    json res = {
        {"error", ErrorCode::None}
    };

    json rs;
    for(auto p: players)
    {
        rs.push_back({
            {"user_id", p->user_id}, 
            {"name", p->name},
            {"state", p->state},
        });
    }

    return res;
}

void Model::PlayerList::Print()
{
    for(auto i: players)
    {
        std::cout<<"user_id: "<<i->user_id<<std::endl;
    }
}


Model::RoomList::RoomList()
{

}

Model::RoomList::~RoomList()
{

}

void Model::RoomList::AppendRoom(Model::Room* room)
{
    rooms.push_back(room);
}

void Model::RoomList::DeleteRoom(int room_id)
{
    auto it = std::find_if(rooms.begin(), rooms.end(), 
                        [room_id](Model::Room *r){
                            return r->room_id == room_id;
                        });

    if(it != rooms.end())
    {
        Model::Room *r = *it;
        rooms.erase(it);
        delete r;
    }
}

Model::Room *Model::RoomList::LoadRoomFromTitle(std::string title)
{
    auto it = std::find_if(rooms.begin(), rooms.end(), 
                        [title](Model::Room *r){
                            return r->title == title;
                        });

    if(it != rooms.end())
    {
        Model::Room *r = *it;
        return r;
    }

    return nullptr;
}

Model::Room *Model::RoomList::LoadRoomFromRoomId(int room_id)
{
    auto it = std::find_if(rooms.begin(), rooms.end(), 
                        [room_id](Model::Room *r){
                            return r->room_id == room_id;
                        });

    if(it != rooms.end())
    {
        Model::Room *r = *it;
        return r;
    }

    return nullptr;
}

void Model::RoomList::LogoutPlayerInRoom(uuid_t user_id, int room_id)
{
    Model::Room *room = LoadRoomFromRoomId(room_id);
    if(room == nullptr)
    {
        return;
    }

    if(room->state == RoomState::RoomReady)
    {
        DeleteRoom(room_id);
    }
    else if(user_id == room->host_id)
    {
        room->host_id = room->other_id;
        room->other_id = 0;
        room->state = RoomState::RoomReady;
    }
}

json Model::RoomList::LoadAllRooms()
{
    json res = {
        {"error", ErrorCode::None}
    };

    json rs;

    for(auto r: rooms)
    {
        rs.push_back({
            {"room_id", r->room_id},
            {"state", r->state}, 
            {"host_id", r->host_id}, 
            {"other_id", r->other_id}, 
            {"title", r->title}, 
            {"is_start", r->is_start}, 
        });
    }
    
    res["rooms"] = rs;
    return res;
}

void Model::RoomList::Print()
{
    for(auto i: rooms)
    {
        std::cout<<"room_title: "<<i->title<<std::endl;
    }
}


Model::Response::Response(socket_t &f, json &r)
: fd(f), res(r)
{

}