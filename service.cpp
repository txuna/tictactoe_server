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

std::tuple<ErrorCode, uuid_t> Service::AccountService::InsertAccount(std::string email, std::string password, std::string salt, std::string name)
{
    if(db_connection.IsOpen() == false)
    {
        return std::make_tuple(ErrorCode::MysqlConnectionClose, 0);
    }

    try
    {
        uuid_t user_id; 

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
    uuid_t user_id;
    std::stringstream email; 
    std::stringstream password;
    std::stringstream salt; 
    std::stringstream name; 
    int permission; 

    if(db_connection.IsOpen() == false)
    {
        return std::make_tuple(ErrorCode::MysqlConnectionClose, nullptr);
    }

    try
    {
        mysqlx::Table table = db_connection.GetTable("accounts");
        mysqlx::RowResult result = table.select("user_id", "email", "password", "salt", "name", "permission")
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
        permission = row[5];

        Model::Account *account = new Model::Account(user_id, 
                                                    email.str(), 
                                                    password.str(), 
                                                    salt.str(), 
                                                    name.str(),
                                                    permission);

        return std::make_tuple(ErrorCode::None, account);
    }
    catch(const mysqlx::Error &err)
    {
        return std::make_tuple(ErrorCode::MysqlError, nullptr);
    }
}

std::tuple<ErrorCode, Model::Account*> Service::AccountService::LoadAccountFromUserId(uuid_t req_user_id)
{
    uuid_t user_id;
    std::stringstream email; 
    std::stringstream password;
    std::stringstream salt; 
    std::stringstream name; 
    int permission;

    if(db_connection.IsOpen() == false)
    {
        return std::make_tuple(ErrorCode::MysqlConnectionClose, nullptr);
    }

    try
    {
        mysqlx::Table table = db_connection.GetTable("accounts");
        mysqlx::RowResult result = table.select("user_id", "email", "password", "salt", "name", "permission")
        .where("user_id = :req_user_id")
        .bind("req_user_id", user_id).execute();

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
        permission = row[5];

        Model::Account *account = new Model::Account(user_id, 
                                                    email.str(), 
                                                    password.str(), 
                                                    salt.str(), 
                                                    name.str(),
                                                    permission);

        return std::make_tuple(ErrorCode::None, account);
    }
    catch(const mysqlx::Error &err)
    {
        return std::make_tuple(ErrorCode::MysqlError, nullptr);
    }
}

ErrorCode Service::AccountService::DeleteUser(uuid_t user_id)
{
    if(db_connection.IsOpen() == false)
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
ErrorCode Service::PlayerService::CreatePlayer(uuid_t user_id)
{
    if(db_connection.IsOpen() == false)
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

ErrorCode Service::PlayerService::UpdatePlayer(uuid_t user_id, Model::DatabaseUser *user)
{
    if(db_connection.IsOpen() == false)
    {
        return ErrorCode::MysqlConnectionClose;
    }

    try
    {
        mysqlx::Table table = db_connection.GetTable("players");
        table.update()
        .set("win", user->win)
        .set("lose", user->lose)
        .set("draw", user->draw)
        .set("point", user->point)
        .where("user_id = :user_id")
        .bind("user_id", user->user_id)
        .execute();

        return ErrorCode::None;
    }
    catch(const mysqlx::Error &err)
    {
        return ErrorCode::MysqlError;
    }
}

std::tuple<ErrorCode, Model::DatabaseUser*> Service::PlayerService::LoadPlayer(uuid_t user_id)
{
    if(db_connection.IsOpen() == false)
    {
        return std::make_tuple(ErrorCode::MysqlConnectionClose, nullptr);
    }

    int win, lose, draw, point;

    try
    {
        mysqlx::Table table = db_connection.GetTable("players");
        mysqlx::RowResult result = table.select("user_id", "win", "lose", "draw", "point")
        .where("user_Id = :user_id")
        .bind("user_id", user_id).execute();

        mysqlx::Row row = result.fetchOne();

        if(row.isNull())
        {
            return std::make_tuple(ErrorCode::NoneExistPlayer, nullptr);
        }

        user_id = row[0];
        win = row[1];
        lose = row[2];
        draw = row[3];
        point = row[4];

        Model::DatabaseUser *user = new Model::DatabaseUser(user_id, 
                                                            win, 
                                                            lose, 
                                                            draw, 
                                                            point);

        return std::make_tuple(ErrorCode::None, user);
    }
    catch(const mysqlx::Error &err)
    {
        return std::make_tuple(ErrorCode::MysqlError, nullptr);
    }
    
}