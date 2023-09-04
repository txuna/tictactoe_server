
CPPFLAGS = -I /usr/include/mysql-cppconn-8/  -I /usr/include/openssl
LDLIBS = -lmysqlcppconn8 -lcrypto

all: server

server: main.o sock.o tevents.o tmysql.o service.o controller.o model.o game.o utility.o
	g++ -g -o server -g main.o sock.o tevents.o tmysql.o service.o controller.o model.o game.o utility.o $(LDLIBS)

main.o: main.h main.cpp
	g++ $(CPPFLAGS) -c -g -o main.o main.cpp

sock.o: sock.h sock.cpp 
	g++ $(CPPFLAGS) -c -g -o sock.o sock.cpp

tevents.o: tevents.h tevents.cpp
	g++ $(CPPFLAGS) -c -g -o tevents.o tevents.cpp

tmysql.o: tmysql.h tmysql.cpp
	g++ $(CPPFLAGS) -c -g -o tmysql.o tmysql.cpp 

service.o: service.h service.cpp
	g++ $(CPPFLAGS) -c -g -o service.o service.cpp

controller.o: controller.h controller.cpp
	g++ $(CPPFLAGS) -c -g -o controller.o controller.cpp

model.o: model.h model.cpp
	g++ $(CPPFLAGS) -c -g -o model.o model.cpp

game.o: game.h game.cpp
	g++ $(CPPFLAGS) -c -g -o game.o game.cpp

utility.o: utility.h utility.cpp
	g++ $(CPPFLAGS) -c -g -o utility.o utility.cpp

clean:
	rm *.o server