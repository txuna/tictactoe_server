# TicTacToe Game(Server and Client)
tictactoe를 즐길 수 있는 클라이언트와 서버

# Stack 
Server : C++, Mysql, redis   
Client : 제작예정(Unity예상)  

# Architecture 


# Trade Off
### Q. 자체적인 통신 프로토콜을 제작하지 않고 nlohmann/json + msgpack을 사용한 이유?  
```
json 특성상 자체적인 통신 프로토콜을 만드는 것보다 크기가 크기에 네트워크처리 면에서 느리지만 코드 유지보수 및 생산성 향상 
또한 json을 msgpack으로 변환하여 크기를 최대한 줄임
```

### 


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