#ifndef __MAIN_H_
#define __MAIN_H_

#include <fstream>
#include <string>
#include <sstream>

#include "sock.h"
#include "common.h"

Net::TcpSocket *SetupServer(int port);
int ParseConfig(std::string *user, 
                std::string *password, 
                std::string *mport, 
                std::string *host, 
                std::string *sport,
                std::string *db_name);



#endif