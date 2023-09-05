#include "controller.h"
#include "utility.h"

Controller::Authentication::Authentication(Mysql::DB &dbc, Redis::DB &rc)
: db_connection(dbc), redis_conn(rc)
{
    account_service = new Service::AccountService(dbc, rc);
    player_service = new Service::PlayerService(dbc, rc);
}

Controller::Authentication::~Authentication()
{
    delete account_service;
    delete player_service;
}

/*
    1. load account model from email
    2. check password (set password hash using stored salt)
    3. Create Token and store in memory(user)
*/
void Controller::Authentication::Login(const json& req)
{
    std::string email = req["email"];
    std::string password = req["password"];
    
    Model::Account *account;
    ErrorCode result;
    std::tie(result, account) = account_service->LoadAccount(email);

    if(result != ErrorCode::None)
    {
        std::cout<<"Login Error: "<<result<<std::endl;
        return;
    }

    std::string n_hash = Utility::Security::GenerateHash(password, account->salt);
    if(n_hash != account->password)
    {
        std::cout<<"Incorreted Password"<<std::endl;
        return;
    }

    std::cout<<"Logined!"<<std::endl;
    std::cout<<Utility::Security::GenerateToken()<<std::endl;

    delete account;
    return;
}

/*
    1. check alreayd has account 
    2. check duplicated email 
    3. insert account 
*/
void Controller::Authentication::Register(const json& req)
{
    std::string email = "test1234@naver.com"; 
    std::string password = "hello"; 
    std::string name = "tuuna";
    std::string salt = Utility::Security::GenerateSalt(24);
    std::string hash = Utility::Security::GenerateHash("hello", salt);

    ErrorCode result;
    uint64_t user_id; 

    std::tie(result, user_id) = account_service->InsertAccount(email, hash, salt, name);

    if(result != ErrorCode::None)
    {
        std::cout<<"Insert Error: "<<result<<std::endl;
        return;
    }

    std::cout<<"Register Account"<<std::endl;
    std::cout<<"user id: "<<user_id<<std::endl;

    result = player_service->CreatePlayer(user_id);
    if(result != ErrorCode::None)
    {
        std::cout<<"Create Player Error: "<<std::endl;
        std::cout<<"delete account"<<std::endl;

        result = account_service->DeleteUser(user_id);
        return;
    }

    return;
}

void Controller::Authentication::Logout(const json& req)
{
    
}