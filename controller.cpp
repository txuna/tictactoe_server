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

    4. store user info in redis

    추후 Response형태로 만들어러 return 
*/
json Controller::Authentication::Login(const json& req)
{
    json response = {
        {"error", ErrorCode::None},
        {"token", ""},
        {"user_id", 0}
    };

    if(req.contains("email") == false 
    || req.contains("password") == false)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }

    std::string email = req["email"];
    std::string password = req["password"];
    std::string token;
    
    Model::Account *account;
    ErrorCode result;
    std::tie(result, account) = account_service->LoadAccount(email);

    if(result != ErrorCode::None)
    {
        response["error"] = result;
        return response;
    }

    std::string n_hash = Utility::Security::GenerateHash(password, account->salt);
    if(n_hash != account->password)
    {
        response["error"] = ErrorCode::InvalidPassword;
        return response;
    }

    token = Utility::Security::GenerateToken();

    result = StoreUserInRedis(account, token);
    if(result != ErrorCode::None)
    {
        response["error"] = result;
        return response;
    }

    response["token"] = token;
    response["user_id"] = account->user_id;

    delete account;
    return response;
}

/*
    1. 필드 확인

    1. check alreayd has account 
    2. check duplicated email 
    3. insert account 
*/
json Controller::Authentication::Register(const json& req)
{
    json response = {
        {"error", ErrorCode::None}
    };

    if(req.contains("email") == false
    || req.contains("password") == false
    || req.contains("name") == false)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }

    std::string email = req["email"]; 
    std::string password = req["password"]; 
    std::string name = req["name"];
    std::string salt = Utility::Security::GenerateSalt(24);
    std::string hash = Utility::Security::GenerateHash(password, salt);

    ErrorCode result;
    uuid_t user_id; 

    std::tie(result, user_id) = account_service->InsertAccount(email, hash, salt, name);

    if(result != ErrorCode::None)
    {
        response["error"] = ErrorCode::AlreadyExistEmail;
        return response;
    }

    result = player_service->CreatePlayer(user_id);
    if(result != ErrorCode::None)
    {
        result = account_service->DeleteUser(user_id);
        response["error"] = ErrorCode::MysqlError;
        return response;
    }

    std::cout << std::setw(4) << response << '\n';

    return response;
}

void Controller::Authentication::Logout(const json& req)
{
    
}

ErrorCode Controller::Authentication::StoreUserInRedis(Model::Account *account, std::string token)
{
    json j = {
        {"user_id", account->user_id}, 
        {"email", account->email},
        {"name", account->name},
        {"token", token}
    };

    std::string key = std::to_string(account->user_id) + "_user";
    std::string j_str = to_string(j);

    ErrorCode result = redis_conn.StoreString(key ,j_str);
    return result;
}