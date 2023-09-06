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

    if(req["email"].type() != json::value_t::string
    || req["password"].type() != json::value_t::string)
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
    4. check duplicate name
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
    
    if(req["email"].type() != json::value_t::string
    || req["password"].type() != json::value_t::string
    || req["name"].type() != json::value_t::string)
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

// redis에서 유저 삭제
json Controller::Authentication::Logout(const json& req)
{
    json response = {
        {"error", ErrorCode::None}
    };

    uuid_t user_id = req["user_id"];

    ErrorCode result = redis_conn.DelKey(std::to_string(user_id)+"_user");
    response["error"] = result;

    return response;
}

ErrorCode Controller::Authentication::StoreUserInRedis(Model::Account *account, std::string token)
{
    json j = {
        {"user_id", account->user_id}, 
        {"email", account->email},
        {"name", account->name},
        {"token", token},
        {"state", PlayerState::Lobby}
    };

    std::string key = std::to_string(account->user_id) + "_user";
    std::string j_str = to_string(j);

    ErrorCode result = redis_conn.StoreString(key ,j_str);
    return result;
}




/*
==========================================================================
*/




Controller::RoomController::RoomController(Mysql::DB &dbc, Redis::DB &rc)
: db_connection(dbc), redis_conn(rc)
{
    player_service = new Service::PlayerService(dbc, rc);
}

Controller::RoomController::~RoomController()
{

}

/*
    Verify Duplicate Title
    플레이어가 이미 PLAYING중인지 확인
    플레이어 상태 변경
*/
std::tuple<json, Model::Room*> Controller::RoomController::CreateRoom(const json &req, int *room_index)
{
    ErrorCode result = ErrorCode::None;
    json response = {
        {"error", result}
    }; 

    if(req.contains("title") == false)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return std::make_tuple(response, nullptr);
    }

    if(req["title"].type() != json::value_t::string)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return std::make_tuple(response, nullptr);
    }

    uuid_t user_id = req["user_id"]; 
    std::string title = req["title"];
    std::string load_str; 

    // 중복 방 검사
    std::tie(result, load_str) = redis_conn.LoadString(title+"_room");
    // DUPLICATE!!!
    if(result == ErrorCode::None)
    {
        response["error"] = ErrorCode::DuplicatedRoomTitle;
        return std::make_tuple(response, nullptr);
    }

    // 플레이어 상태 확인 
    json player_j;
    std::tie(result, player_j) = LoadPlayer(user_id);

    if(result != ErrorCode::None)
    {
        response["error"] = result;
        return std::make_tuple(response, nullptr);
    }
    
    if(player_j["state"] == PlayerState::Playing)
    {
        response["error"] = ErrorCode::AlreadyPlaying;
        return std::make_tuple(response, nullptr);
    }

    // player 상태 변경 
    result = ChangePlayer(user_id, player_j, PlayerState::Playing);
    if(result != ErrorCode::None)
    {
        return std::make_tuple(response, nullptr);
    }

    Model::Room* room = new Model::Room(user_id, RoomState::RoomWaiting, title, *room_index);

    result = StoreRoomInRedis(room, title+"_room");
    // @@TODO 플레이어 상태 롤백 
    if(result != ErrorCode::None)
    {
        delete room;
        response["error"] = result;
        return std::make_tuple(response, nullptr);
    }

    *room_index += 1;
    return std::make_tuple(response, room);
}

// 자신이 이미 방에 들어가 있는지 확인 
// 방이 실제로 존재하며 WAITING 상태인지 확인 
std::tuple<json, uuid_t> Controller::RoomController::JoinRoom(const json &req)
{
    json response = {
        {"error", ErrorCode::None}
    }; 

    uuid_t user_id = req["user_id"];

    if(req.contains("title") == false)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return std::make_tuple(response, 0); 
    }
    
    if(req["title"].type() == json::value_t::string)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return std::make_tuple(response, 0);
    }

    std::string title = req["title"];
    std::string load_str; 
    ErrorCode result;
    json player_j;
    
    // 요청한 방이 있는지 확인
    std::tie(result, load_str) = redis_conn.LoadString(title+"_room"); 
    if(result != ErrorCode::None)
    {
        response["error"] = ErrorCode::NoneExistRoom;
        return std::make_tuple(response, 0); 
    }

    // player 정보도 가지고 와서 현재 참여중인지 확인 
    std::tie(result, player_j) = LoadPlayer(user_id);
    if(result != ErrorCode::None)
    {
        response["error"] = ErrorCode::NoneExistPlayer;
        return std::make_tuple(response, 0);
    }

    return std::make_tuple(response, user_id);
}

json Controller::RoomController::StartRoom(const json &req)
{
    json response = {
        {"error", ErrorCode::None}
    }; 

    return response;
}

json Controller::RoomController::ExitRoom(const json &req)
{
    json response = {
        {"error", ErrorCode::None}
    }; 

    return response;
}

json Controller::RoomController::LoadRoom(const json &req)
{
    json response = {
        {"error", ErrorCode::None}
    }; 

    return response;
}

ErrorCode Controller::RoomController::StoreRoomInRedis(Model::Room *room, std::string key)
{

    json j = {
        {"host_user_id", room->host_user_id}, 
        {"other_user_Id", room->other_user_id},
        {"room_state", room->room_state}, 
        {"room_title", room->room_title}, 
        {"room_id", room->room_id}
    };

    std::string j_str = to_string(j);
    ErrorCode result = redis_conn.StoreString(key, j_str);
    return result;
}

ErrorCode Controller::RoomController::ChangePlayer(uuid_t user_id, json player_j, PlayerState state)
{
    ErrorCode result;
    player_j["state"] = state;
    result = redis_conn.StoreString(std::to_string(user_id)+"_user", to_string(player_j));
    if(result != ErrorCode::None)
    {
        return ErrorCode::RedisError;
    }

    return ErrorCode::None;
}


std::tuple<ErrorCode, json> Controller::RoomController::LoadPlayer(uuid_t user_id)
{   
    ErrorCode result;
    std::string load_str;

    std::tie(result, load_str) = redis_conn.LoadString(std::to_string(user_id)+"_user");
    if(result != ErrorCode::None)
    {
        return std::make_tuple(ErrorCode::NoneExistPlayer, 0);
    }

    // Player 상태 변경 후 다시 저장 
    json player_j = json::parse(load_str);
    return std::make_tuple(ErrorCode::None, player_j);
}