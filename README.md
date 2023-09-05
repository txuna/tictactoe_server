# TicTacToe Game(Server and Client)
tictactoe를 즐길 수 있는 클라이언트와 서버

# Stack 
Server : C++, Mysql, redis  
Client : 제작중(Unity예상)  

# Architecture 




# Dependency
1. [nlohmann/json.hpp](https://github.com/nlohmann/json)  
2. [mysql connector/c++ 8.0(up)](https://github.com/mysql/mysql-connector-cpp)  
3. [openssl 3.0(up)](https://github.com/openssl/openssl)  
4. [hiredis](https://github.com/redis/hiredis)   

# Compile 
```
git clone https://github.com/txuna/tictactoe_server.git
cd tictactoe_server
make
./server
```

# Config 
Create config file

touch config.conf
```
server_port=0000
user=0000
password=0000
mysql_port=0000
host=127.0.0.1
database=0000
```