#!/bin/bash

CC=gcc
CXX=g++
CFLAGS= -std=c++11 -ggdb
LIBS= -lpthread -pthread -lcurl

all: server

server: ./obj/main.o ./obj/ClientInfo.o ./obj/Server.o ./obj/Users.o
	$(CXX) $(CFLAGS) ./obj/main.o ./obj/ClientInfo.o ./obj/Server.o ./obj/Users.o $(LIBS) -o server 

./obj/main.o: ./src/main.cpp ./inc/Server.h ./inc/Users.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/main.cpp $(LIBS) -o ./obj/main.o
	
./obj/Users.o: ./src/Users.cpp ./inc/Users.h ./inc/json.hpp
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Users.cpp $(LIBS) -o ./obj/Users.o

./obj/Server.o: ./src/Server.cpp ./inc/Server.h ./inc/ClientInfo.h
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Server.cpp $(LIBS) -o ./obj/Server.o

./obj/ClientInfo.o: ./src/ClientInfo.cpp ./inc/ClientInfo.h ./inc/Users.h 
	$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/ClientInfo.cpp -o ./obj/ClientInfo.o

clean:
	rm -rf ./obj/*.o server

rebuild_all: clean all
	
