#include "tmysql.h"


Mysql::DB::DB(std::string host, 
              std::string port, 
              std::string user, 
              std::string password)
{
    this->host = host; 
    this->port = port; 
    this->user = user; 
    this->password = password;

    from_options = nullptr; 
    sess = nullptr;
    sch = nullptr;

    is_close = true;
}

bool Mysql::DB::Connect(std::string database)
{
    try
    {
        from_options = new mysqlx::SessionSettings(host, stoi(port), user, password);
        sess = new mysqlx::Session(*from_options);
        sch = new mysqlx::Schema(sess->getSchema(database));
        is_close = false;
        return true;
    }
    catch(const mysqlx::Error &err)
    {
        std::cout<<"DB Error : "<<err<<std::endl;
        return false;
    }
}

mysqlx::Table Mysql::DB::GetTable(std::string tname)
{
    return sch->getTable(tname);
}


bool Mysql::DB::IsOpen()
{
    return !is_close;
}

Mysql::DB::~DB()
{
    if(is_close)
    {
        return;
    }
    
    is_close = true; 
    sess->close();

    if(
        from_options == nullptr 
        || sess == nullptr
        || sch == nullptr)
    {
        return;
    }
    
    delete from_options; 
    delete sess;
    delete sch;
}