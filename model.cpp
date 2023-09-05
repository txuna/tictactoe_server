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

Model::RedisUser::RedisUser(std::string n, std::string token, uuid_t u)
{

}

Model::RedisUser::~RedisUser()
{

}