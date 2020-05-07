#!/bin/bash
CC=gcc
CXX=g++
CFLAGS= -std=c++11 -ggdb
LIBS= -lpthread -pthread -lcurl -lev -lcrypto -lssl -lmysqlcppconn
OBJS= ./obj/main.o ./obj/Server.o ./obj/RegistryService.o ./obj/utilities.o
OBJS+= ./obj/Protocol.o ./obj/Requests.o ./obj/Jwt.o ./obj/base64.o
OBJS+= ./obj/Matcher.o ./obj/Game.o ./obj/GameService.o ./obj/Questions.o
OBJS+= ./obj/debug.o ./obj/GameDAL.o ./obj/RegistryDAO.o ./obj/Timer.o
OBJS+= ./obj/QuestionsDAO.o ./obj/UserStatisticsDAO.o

all: server

server: build image

image:
	@echo "linking $@"
	@$(CXX) $(CFLAGS) $(OBJS) $(LIBS) -o server

build: main RegistryService Server Jwt base64 Protocol Requests utilities Matcher Game GameService Questions debug GameDAL RegistryDAO Timer QuestionsDAO UserStatisticsDAO

main: ./src/main.cpp
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" -I"./version" ./src/main.cpp -o ./obj/main.o
	
RegistryService: ./src/RegistryService.cpp ./inc/RegistryService.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/RegistryService.cpp -o ./obj/RegistryService.o

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

Matcher: ./src/Matcher.cpp ./inc/Matcher.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Matcher.cpp -o ./obj/Matcher.o

Game: ./src/Game.cpp ./inc/Game.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Game.cpp -o ./obj/Game.o

GameService: ./src/GameService.cpp ./inc/GameService.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/GameService.cpp -o ./obj/GameService.o

Questions: ./src/Questions.cpp ./inc/Questions.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Questions.cpp -o ./obj/Questions.o

debug: ./src/debug.cpp ./inc/debug.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/debug.cpp -o ./obj/debug.o

GameDAL: ./src/GameDAL.cpp ./inc/GameDAL.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/GameDAL.cpp -o ./obj/GameDAL.o

RegistryDAO: ./src/RegistryDAO.cpp ./inc/RegistryDAO.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/RegistryDAO.cpp -o ./obj/RegistryDAO.o

QuestionsDAO: ./src/QuestionsDAO.cpp ./inc/QuestionsDAO.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/QuestionsDAO.cpp -o ./obj/QuestionsDAO.o

Timer: ./src/Timer.cpp ./inc/Timer.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/Timer.cpp -o ./obj/Timer.o

UserStatisticsDAO: ./src/UserStatisticsDAO.cpp ./inc/UserStatisticsDAO.h
	@echo "compiling $@"
	@$(CXX) $(CFLAGS) -c -g -I"./inc" ./src/UserStatisticsDAO.cpp -o ./obj/UserStatisticsDAO.o
	
incver:
	@./version/incver.sh

clean:
	@echo "Cleaning project..."
	@rm -rf ./obj/*.o server

rebuild_all: clean build
	
