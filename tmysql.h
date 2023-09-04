#ifndef __MYSQL_H
#define __MYSQL_H

#include <iostream>
#include <list>
#include <string>
#include <mysqlx/xdevapi.h>

namespace Mysql
{
    class DB
    {
        private:
            std::string host;
            std::string port; 
            std::string user; 
            std::string password;

            mysqlx::SessionSettings *from_options; 
            mysqlx::Session *sess; 
            mysqlx::Schema *sch;

            bool is_close;

        public:
            DB(std::string host, std::string port, std::string user, std::string password);
            ~DB();
            bool Connect(std::string database);
            bool IsOpen();
            mysqlx::Table GetTable(std::string tname);
    };
}

#endif 