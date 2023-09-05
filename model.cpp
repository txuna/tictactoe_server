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

Model::Player::Player(uuid_t u, socket_t f)
: user_id(u), fd(f)
{

}

Model::Player::~Player()
{

}