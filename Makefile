#!/bin/bash

CC=gcc
CXX=g++
CFLAGS= -std=c++11 -ggdb
LIBS= -lpthread -pthread -lcurl -lcrypto -lssl

all: server

server: ./obj/main.o ./obj/ClientInfo.o ./obj/Server.o ./obj/Users.o
	$(CXX) $(CFLAGS) ./obj/main.o ./obj/ClientInfo.o ./obj/Server.o ./obj/Users.o $(LIBS) -o server 

./obj/main.o: ./src/main.cpp ./inc/Server.h ./inc/Users.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/main.cpp -o ./obj/main.o
	
./obj/Users.o: ./src/Users.cpp ./inc/Users.h ./inc/json.hpp
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Users.cpp -o ./obj/Users.o

./obj/Server.o: ./src/Server.cpp ./inc/Server.h ./inc/ClientInfo.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Server.cpp -o ./obj/Server.o

./obj/ClientInfo.o: ./src/ClientInfo.cpp ./inc/ClientInfo.h ./inc/Users.h 
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/ClientInfo.cpp -o ./obj/ClientInfo.o

./obj/Jwt.o: ./src/Jwt.cpp ./inc/Jwt.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Jwt.cpp -o ./obj/Jwt.o

./obj/base64.o: ./src/base64.cpp ./inc/base64.h 
	g++ $(CFLAGS) ./src/base64.cpp -o ./obj/base64.o

clean:
	rm -rf ./obj/*.o server

rebuild_all: clean all
	
