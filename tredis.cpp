#include "tredis.h"


Redis::DB::DB(std::string h, std::string p)
: host(h), port(p)
{
    expire_time = 3600;
}

Redis::DB::~DB()
{
    if(conn != nullptr)
    {
        redisFree(conn);
    }
}

bool Redis::DB::Connect()
{
    if(conn != nullptr)
    {
        return false;
    }

    conn = redisConnect(host.c_str(), stoi(port));

    if(conn != NULL && conn->err)
    {
        return false;
    }

    return true;
}

ErrorCode Redis::DB::StoreString(std::string key, std::string value)
{
    redisReply *reply = (redisReply*)redisCommand(conn, "SET %s %s EX %d", key.c_str(), value.c_str(), expire_time);
    ErrorCode result = ErrorCode::None;

    if(reply->type == REDIS_REPLY_ERROR)
    {
        result = ErrorCode::RedisError;
    }

    freeReplyObject(reply);
    return result;
}

std::tuple<ErrorCode, std::string> Redis::DB::LoadString(std::string key)
{
    redisReply *reply = (redisReply*)redisCommand(conn, "GET %s", key);
    
    if(reply->type == REDIS_REPLY_ERROR)
    {
        return std::make_tuple(ErrorCode::RedisError, "");
    }

    freeReplyObject(reply);
    return std::make_tuple(ErrorCode::None, reply->str);
}