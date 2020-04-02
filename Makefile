#!/bin/bash
CC=gcc
CXX=g++
CFLAGS= -std=c++11 -ggdb
LIBS= -lpthread -pthread -lcurl -lev -lcrypto -lssl
OBJS= ./obj/main.o ./obj/Server.o ./obj/Users.o ./obj/utilities.o
OBJS+= ./obj/Protocol.o ./obj/Requests.o ./obj/Jwt.o ./obj/base64.o

all: server

server: build image

image:
	@echo "linking $@"
	@$(CXX) $(CFLAGS) $(OBJS) $(LIBS) -o server

build: main Users Server Jwt base64 Protocol Requests utilities

main: ./src/main.cpp ./inc/Server.h ./inc/Users.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/main.cpp -o ./obj/main.o
	
Users: ./src/Users.cpp ./inc/Users.h ./inc/json.hpp
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Users.cpp -o ./obj/Users.o

Server: ./src/Server.cpp ./inc/Server.h ./inc/Requests.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Server.cpp -lev -o ./obj/Server.o

Jwt: ./src/Jwt.cpp ./inc/Jwt.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Jwt.cpp -o ./obj/Jwt.o

base64: ./src/base64.cpp ./inc/base64.h 
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/base64.cpp -o ./obj/base64.o

Protocol: ./src/Protocol.cpp ./inc/Protocol.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Protocol.cpp -o ./obj/Protocol.o

Requests: ./src/Requests.cpp ./inc/Requests.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Requests.cpp -o ./obj/Requests.o

utilities: ./src/utilities.cpp ./inc/utilities.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/utilities.cpp -o ./obj/utilities.o

clean:
	rm -rf ./obj/*.o server

rebuild_all: clean build
	
