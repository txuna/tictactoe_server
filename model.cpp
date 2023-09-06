#include "model.h"

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

Model::Player::Player(uuid_t u, socket_t f, PlayerState s)
: user_id(u), fd(f), state(s)
{

}

Model::Player::~Player()
{

}

Model::Room::Room(uuid_t h, RoomState rs, std::string rt, int ri)
: host_user_id(h), room_state(rs), room_title(rt), room_id(ri), other_user_id(0)
{
    
}

Model::Room::~Room()
{

}