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
json Controller::Authentication::Login(const json &req, socket_t fd, Model::PlayerList &players)
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

    response["token"] = token;
    response["user_id"] = account->user_id;

    /* 플레이어 추가 */
    players.AppendPlayer(account->user_id, fd, PlayerState::Lobby, token);

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

/* players에서 제거 */
/* 해당 player가 Room에 있는지 확인 */
/* 있다면 host인지 other인지 확인 */
/* 게임 중인지 아닌지도 중요 */
json Controller::Authentication::Logout(const json& req, Model::PlayerList &players, Model::RoomList &rooms)
{
    json response = {
        {"error", ErrorCode::None}
    };

    uuid_t user_id = req["user_id"];

    // 플레이어의 상태 확인 
    Model::Player *player = players.LoadPlayer(user_id);
    if(player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistPlayer;
        return response;
    }

    // host, other 둘다 한번에 확인이 가능한가? 

    // 참여중인 방 확인 
    // 호스트라면 삭제
    

    players.DeletePlayerFromUserId(user_id);
    return response;
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
json Controller::RoomController::CreateRoom(const json &req, Model::PlayerList &players, Model::RoomList &rooms)
{
    ErrorCode result = ErrorCode::None;
    json response = {
        {"error", result}
    }; 

    if(req.contains("title") == false)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }

    if(req["title"].type() != json::value_t::string)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }

    uuid_t user_id = req["user_id"]; 
    std::string title = req["title"];

    // 중복 방 검사
    Model::Room *r = rooms.LoadRoomFromTitle(title);
    if(r != nullptr)
    {
        response["error"] = ErrorCode::DuplicatedRoomTitle;
        return response;
    }

    // 플레이어 상태 확인 
    Model::Player *player = players.LoadPlayer(user_id);
    if(player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistPlayer;
        return response;
    }

    Model::Room* room = new Model::Room(user_id, RoomState::RoomReady, title, rooms.room_index);
    rooms.room_index += 1;
    rooms.AppendRoom(room);

    // player 상태 변경 및 방 아이디 세팅 
    player->state = PlayerState::Playing;
    player->room_id = room->room_id;

    return response;
}

// 자신이 이미 방에 들어가 있는지 확인 
// 방이 실제로 존재하며 WAITING 상태인지 확인 
json Controller::RoomController::JoinRoom(const json &req, Model::PlayerList &players, Model::RoomList &rooms)
{
    json response = {
        {"error", ErrorCode::None}
    }; 

    uuid_t user_id = req["user_id"];

    if(req.contains("room_id") == false)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }
    
    if(req["room_id"].type() != json::value_t::number_integer)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }

    int room_id = req["room_id"];

    // 요청한 방이 있는지 확인
    Model::Room *room = rooms.LoadRoomFromRoomId(room_id);
    if(room == nullptr)
    {
        response["error"] = ErrorCode::NoneExistRoom;
        return response;
    }

    // 방의 상태 확인
    if(room->state == RoomState::RoomPlaying)
    {
        response["error"] = ErrorCode::AlreadyRoomPlaying;
        return response;
    }

    Model::Player *player = players.LoadPlayer(user_id);
    if(player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistPlayer;
        return response;
    }

    // player 상태 확인
    if(player->state == PlayerState::Playing)
    {
        response["error"] = ErrorCode::AlreadyPlaying;
        return response;
    }

    // 플레이어랑 방 정보 변경 및 저장 
    player->state = PlayerState::Playing;
    player->room_id = room_id;
    room->other_id = user_id;

    return response;
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