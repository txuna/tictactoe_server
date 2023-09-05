#include "service.h"
#include <iostream>
#include <sstream>

Service::AccountService::AccountService(Mysql::DB &dbc, Redis::DB &rc)
: db_connection(dbc), redis_conn(rc)
{

}

Service::AccountService::~AccountService()
{
    
}

std::tuple<ErrorCode, uint64_t> Service::AccountService::InsertAccount(std::string email, std::string password, std::string salt, std::string name)
{
    if(!db_connection.IsOpen())
    {
        return std::make_tuple(ErrorCode::MysqlConnectionClose, 0);
    }

    try
    {
        uint64_t user_id; 

        mysqlx::Table table = db_connection.GetTable("accounts");
        mysqlx::Result result = table.insert("email", "password", "salt", "name")
        .values(email, password, salt, name)
        .execute();

        user_id = result.getAutoIncrementValue();

        return std::make_tuple(ErrorCode::None, user_id);
    }
    catch(const mysqlx::Error &err)
    {
        return std::make_tuple(ErrorCode::MysqlError, 0);
    }
}

std::tuple<ErrorCode, Model::Account*> Service::AccountService::LoadAccount(std::string req_email)
{
    int user_id;
    std::stringstream email; 
    std::stringstream password;
    std::stringstream salt; 
    std::stringstream name; 

    if(!db_connection.IsOpen())
    {
        return std::make_tuple(ErrorCode::MysqlConnectionClose, nullptr);
    }

    try
    {
        mysqlx::Table table = db_connection.GetTable("accounts");
        mysqlx::RowResult result = table.select("user_id", "email", "password", "salt", "name")
        .where("email = :email")
        .bind("email", req_email).execute();

        mysqlx::Row row = result.fetchOne();

        if(row.isNull())
        {
            return std::make_tuple(ErrorCode::NoneExistAccount, nullptr);
        }

        user_id = row[0];
        email << row[1];
        password << row[2];
        salt << row[3];
        name << row[4];

        Model::Account *account = new Model::Account(user_id, 
                                                    email.str(), 
                                                    password.str(), 
                                                    salt.str(), 
                                                    name.str());

        return std::make_tuple(ErrorCode::None, account);
    }
    catch(const mysqlx::Error &err)
    {
        return std::make_tuple(ErrorCode::MysqlError, nullptr);
    }
}

ErrorCode Service::AccountService::DeleteUser(uint64_t user_id)
{
    if(!db_connection.IsOpen())
    {
        return ErrorCode::MysqlConnectionClose;
    }

    try
    {
        mysqlx::Table table = db_connection.GetTable("accounts");
        table.remove()
        .where("user_id = :user_id")
        .bind("user_id", user_id).execute();
         
        return ErrorCode::None;
    }
    catch(const mysqlx::Error &err)
    {
        return ErrorCode::MysqlError;
    }
}


Service::PlayerService::PlayerService(Mysql::DB &dbc, Redis::DB &rc)
: db_connection(dbc), redis_conn(rc)
{

}

Service::PlayerService::~PlayerService()
{

}

//@@TODO : ErrorCode확인하고 None이 아니라면 accounts - user_id에 해당하는거 삭제
ErrorCode Service::PlayerService::CreatePlayer(uint64_t user_id)
{
    if(!db_connection.IsOpen())
    {
        return ErrorCode::MysqlConnectionClose;
    }

    try
    {
        mysqlx::Table table = db_connection.GetTable("players");
        mysqlx::Result result = table.insert("user_id", "win", "lose", "draw", "point")
        .values(user_id, 0, 0, 0, 1000)
        .execute();

        return ErrorCode::None;
    }
    catch(const mysqlx::Error &err)
    {
        return ErrorCode::MysqlError;
    }
}