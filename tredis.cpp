#include "tredis.h"


Redis::DB::DB(std::string h, std::string p)
: host(h), port(p)
{

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