#include "controller.h"
#include "utility.h"

Controllers::Controller::Controller(Mysql::DB &dbc, Redis::DB &rc, Model::PlayerList &ps, Model::RoomList &rs, std::queue<Model::Response> &rq)
: db_connection(dbc), redis_conn(rc), players(ps), rooms(rs), res_queue(rq)
{
    account_service = new Service::AccountService(dbc, rc);
    player_service = new Service::PlayerService(dbc, rc);
}

Controllers::Controller::~Controller()
{
    delete account_service;
    delete player_service;
}

/*
    1. load account model from email
    2. check password (set password hash using stored salt)
    3. Create Token and store in memory(user)

    4. store user info in redis

    5. 현재 플레이어목록에 포함되어있다면 막기

    추후 Response형태로 만들어러 return 
*/
json Controllers::Controller::Login(const json &req, socket_t fd)
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

    // 이미 로그인했는지 확인
    Model::Player *player = players.LoadPlayer(account->user_id);
    if(player != nullptr)
    {
        response["error"] = ErrorCode::AlreadyLogined;
        return response;
    }

    token = Utility::Security::GenerateToken();

    response["token"] = token;
    response["user_id"] = account->user_id;

    /* 플레이어 추가 */
    players.AppendPlayer(account->user_id, fd, PlayerState::Lobby, token, account->name);

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
json Controllers::Controller::Register(const json& req)
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

    return response;
}

/* players에서 제거 */
/* 해당 player가 Room에 있는지 확인 */
/* 있다면 host인지 other인지 확인 */
/* 게임 중인지 아닌지도 중요 */
json Controllers::Controller::Logout(const json& req)
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

    if(player->state == PlayerState::Playing)
    {
        // 방이 게임시작중인지 확인 
        Model::Room *room = rooms.LoadRoomFromRoomId(player->room_id);
        if(room != nullptr)
        {
            if(room->is_start == true)
            {
                response["error"] = ErrorCode::CannotExitRoomWithPlaying;
                return response;
            }

            rooms.LogoutPlayerInRoom(user_id, player->room_id);
        }
    }

    players.DeletePlayerFromUserId(user_id);
    return response;
}

json Controllers::Controller::LoadPlayer(const json &req)
{
    json response = players.LoadAllPlayers();
    return response;
}

/*
    Verify Duplicate Title
    플레이어가 이미 PLAYING중인지 확인
    플레이어 상태 변경
*/
json Controllers::Controller::CreateRoom(const json &req)
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

    response["room_id"] = room->room_id;
    return response;
}

// 자신이 이미 방에 들어가 있는지 확인 
// 방이 실제로 존재하며 WAITING 상태인지 확인 
json Controllers::Controller::JoinRoom(const json &req)
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
    
    if(req["room_id"].type() != json::value_t::number_unsigned)
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
        response["error"] = ErrorCode::RoomIsFull;
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
    room->state = RoomState::RoomPlaying;

    Model::Player *host_player = players.LoadPlayer(room->host_id);
    if(host_player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistHostPlayer;
        return response;
    }

    /* 참여하려는 플레이어게 host의 정보를 준다. */
    response["player"] = {
        {"user_id", host_player->user_id}, 
        {"name", host_player->name}
    };

    /* 호스트 플레이어에게 입장하려는 플레이어의 정보를 준다. */
    protocol_t type = ServerMsg::PlayerJoinRoomResponse;
    json j = {
        {"error", ErrorCode::None}, 
        {"player", {
            {"user_id", user_id}, 
            {"name", player->name}
        }}
    };

    res_queue.push(Model::Response(host_player->fd, j, type));
    return response;
}

/* 게임 시작전 방 초기화 */
json Controllers::Controller::StartRoom(const json &req)
{
    json response = {
        {"error", ErrorCode::None}
    }; 

    uuid_t user_id = req["user_id"];

    // 플레이어가 플레이상태인지 확인 
    Model::Player *player = players.LoadPlayer(user_id);
    if(player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistPlayer; 
        return response;
    }

    if(player->state != PlayerState::Playing)
    {
        response["error"] = ErrorCode::PlayerIsNotInRoom;
        return response;
    }

    // 방이 존재하는지 확인 
    Model::Room *room = rooms.LoadRoomFromRoomId(player->room_id);
    if(room == nullptr)
    {
        response["error"] = ErrorCode::NoneExistRoom;
        return response;
    }

    // 호스트인지 확인 
    if(user_id != room->host_id)
    {
        response["error"] = ErrorCode::IsNotHost;
        return response;
    }

    // 방의 상태 확인
    if(room->state != RoomState::RoomPlaying)
    {
        response["error"] = ErrorCode::RoomIsNotFull;
        return response;
    }

    if(room->is_start == true)
    {
        response["error"] = ErrorCode::AlreadyRoomPlaying;
        return response;
    }

    // 다른 other id에게도 게임시작됨을 알림 
    Model::Player *other_player = players.LoadPlayer(room->other_id);
    if(other_player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistOtherPlayer;
        return response;
    }
    
    response["who_is_turn"] = room->host_id;
    res_queue.push(Model::Response(other_player->fd, response, ServerMsg::RoomStartResponse));

    // 방의 상태 변경 및 초기화 
    for(int i=0;i<9;i++)
    {
        room->board[i] = NONE_STONE;
    }
    room->is_start = true;
    room->who_is_turn = room->host_id;
    return response;
}

json Controllers::Controller::ExitRoom(const json &req)
{
    json response = {
        {"error", ErrorCode::None}
    }; 

    uuid_t user_id = req["user_id"];

    // 플레이어 상태 확인
    Model::Player *player = players.LoadPlayer(user_id);
    if(player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistPlayer;
        return response;
    }

    if(player->state != PlayerState::Playing)
    {
        response["error"] = ErrorCode::PlayerIsNotInRoom;
        return response;
    }

    // 방 유무 확인 
    Model::Room *room = rooms.LoadRoomFromRoomId(player->room_id);
    if(room == nullptr)
    {
        response["error"] = ErrorCode::NoneExistRoom;
        return response;
    }

    // room.logout 시전 
    if(room->is_start == true)
    {
        response["error"] = ErrorCode::CannotExitRoomWithPlaying;
        return response;
    }

    rooms.LogoutPlayerInRoom(user_id, player->room_id);

    // host id에게 누군가가 방을 나갔음을 알림 
    // 기존 host이든 other이 host로 바뀌었든 host_id이기에 통일
    Model::Room *r = rooms.LoadRoomFromRoomId(player->room_id);
    if(r != nullptr)
    {
        Model::Player *host_player = players.LoadPlayer(r->host_id);
        if(host_player != nullptr)
        {
            json j = {
                {"error", ErrorCode::None},
                {"user_id", user_id}
            };

            res_queue.push(Model::Response(host_player->fd, j, ServerMsg::PlayerExitRoomResponse));
        }
    }

    return response;
}

json Controllers::Controller::PlayerTurn(const json &req)
{
    json response = {
        {"error", ErrorCode::None}
    };

    if(req.contains("position") == false)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }

    if(req["position"].type() == json::value_t::number_unsigned)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }

    uuid_t user_id = req["user_id"];
    int pos = req["position"]; 

    if(pos < 0 && pos > 8)
    {
        response["error"] = ErrorCode::InvalidRequest;
        return response;
    }

    // 현재 플레이어 및 
    Model::Player *host_player = players.LoadPlayer(user_id);
    if(host_player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistPlayer;
        return response;
    }

    if(host_player->state != PlayerState::Playing)
    {
        response["error"] = ErrorCode::PlayerIsNotInRoom;
        return response;
    }

    Model::Room *room = rooms.LoadRoomFromRoomId(host_player->room_id);
    if(room == nullptr)
    {
        response["error"] = ErrorCode::NoneExistRoom;
        return response;
    }

    // 방 상태 확인
    if(room->state != RoomState::RoomPlaying || room->is_start == false)
    {
        response["error"] = ErrorCode::RoomIsNotStart;
        return response;
    }

    Model::Player *other_player = players.LoadPlayer(room->other_id);
    if(other_player == nullptr)
    {
        response["error"] = ErrorCode::NoneExistOtherPlayer;
        return response;
    }

    // 플레이어의 턴이 맞는지 확인 
    if(user_id == room->who_is_turn)
    {
        response["error"] = ErrorCode::IsNotYourTurn;
        return response;
    }

    // 게임판 3x3 배열에 원하는 위치 기입
    if(room->board[pos] != 0)
    {
        response["error"] = ErrorCode::AlreadyLocatedStone;
        return response;
    }
    
    int win_type = CalculateGame(req, response, room);
    if(win_type != NONE_STONE)
    {
        UpdatePlayerScore(win_type, room);
    }

    res_queue.push(Model::Response(other_player->fd, response, ServerMsg::PlayerTurnResponse));
    
    return response;
}

json Controllers::Controller::LoadRoom(const json &req)
{
    json response = rooms.LoadAllRooms();
    return response;
}


int Controllers::Controller::CalculateGame(const json &req, json &response, Model::Room *room)
{   
    uuid_t req_user_id = req["user_id"];
    int pos = req["position"];

    response["turn_user_id"] = req_user_id;
    response["position"] = pos;
    response["is_fin"] = false;

    if(req_user_id == room->host_id)
    {
        room->board[pos] = HOST_STONE;
        response["who_is_turn"] = room->other_id;
    }
    else
    {
        room->board[pos] = OTHER_STONE;
        response["who_is_tuern"] = room->host_id;
    }

    int win_type = room->CheckWin();

    if(win_type != NONE_STONE)
    {
        response["is_fin"] = true;
        room->is_start = false;

        switch (win_type)
        {
            case HOST_STONE:
            {
                response["who_is_win"] = room->host_id;
                break;
            }
            case OTHER_STONE:
            {
                response["who_is_win"] = room->other_id;
                break;
            }
            case DRAW_STONE:
            {
                response["who_is_win"] = DRAW_GAME;
                break;
            }
        }
    }

    return win_type;
}

void Controllers::Controller::UpdatePlayerScore(int win_type, Model::Room *room)
{
    Model::DatabaseUser *host_user;
    Model::DatabaseUser *other_user; 
    ErrorCode result; 

    std::tie(result, host_user) = player_service->LoadPlayer(room->host_id);
    std::tie(result, other_user) = player_service->LoadPlayer(room->other_id);

    if(result != ErrorCode::None)
    {
        if(host_user != nullptr)
        {
            delete host_user;
        }

        if(other_user != nullptr)
        {
            delete other_user;
        }

        return;
    }

    if(win_type == DRAW_STONE)
    {
        host_user->draw += 1;
        host_user->point += DRAW_POINT;
        other_user->draw += 1; 
        other_user->point += DRAW_POINT;
    }

    else if(win_type == HOST_STONE || win_type == OTHER_STONE)
    {
        Model::DatabaseUser *win_user, *lose_user;

        if(win_type == HOST_STONE)
        {
            win_user = host_user;
            lose_user = other_user;
        }

        else if(win_type == OTHER_STONE)
        {
            win_user = other_user;
            lose_user = host_user;
        }

        win_user->win += 1;
        win_user->point += WIN_POINT;

        lose_user->lose += 1;

        if(lose_user->point - LOSE_POINT < 0)
        {
            lose_user->point = 0;
        }
        else
        {
            lose_user->point -= LOSE_POINT;
        }
    }
}