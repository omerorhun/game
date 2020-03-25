#!/bin/bash

CC=gcc
CXX=g++
CFLAGS= -std=c++11 -ggdb
LIBS= -lpthread -pthread -lcurl -lcrypto -lssl
OBJS= ./obj/main.o ./obj/ClientInfo.o ./obj/Server.o ./obj/Users.o 
OBJS+= ./obj/Protocol.o ./obj/Requests.o ./obj/Jwt.o ./obj/base64.o ./obj/utilities.o

all: server

server: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) $(LIBS) -o server

./obj/main.o: ./src/main.cpp ./inc/Server.h ./inc/Users.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/main.cpp -o ./obj/main.o
	
./obj/Users.o: ./src/Users.cpp ./inc/Users.h ./inc/json.hpp
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Users.cpp -o ./obj/Users.o

./obj/Server.o: ./src/Server.cpp ./inc/Server.h ./inc/ClientInfo.h ./inc/Requests.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Server.cpp -o ./obj/Server.o

./obj/ClientInfo.o: ./src/ClientInfo.cpp ./inc/ClientInfo.h ./inc/Users.h 
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/ClientInfo.cpp -o ./obj/ClientInfo.o

./obj/Jwt.o: ./src/Jwt.cpp ./inc/Jwt.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Jwt.cpp -o ./obj/Jwt.o

./obj/base64.o: ./src/base64.cpp ./inc/base64.h 
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/base64.cpp -o ./obj/base64.o

./obj/Protocol.o: ./src/Protocol.cpp ./inc/Protocol.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Protocol.cpp -o ./obj/Protocol.o

./obj/Requests.o: ./src/Requests.cpp ./inc/Requests.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Requests.cpp -o ./obj/Requests.o

./obj/utilities.o: ./src/utilities.cpp ./inc/utilities.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/utilities.cpp -o ./obj/utilities.o

clean:
	rm -rf ./obj/*.o server

rebuild_all: clean all
	
