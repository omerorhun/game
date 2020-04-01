/*
    Server 23.02.2020
*/

// standart libs
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

// for sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

// for threads
#include <pthread.h>

// for special ops
#include "Server.h"
#include "Users.h"
#include <vector>
#include <map>

// for ipc
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>

#define PORT_NO 1903
#define BUFFER_SIZE 4096

using namespace std;

void *client_operations(void * arg);
void *listen_user(void *arg);
int find_opponent(int id);

void print_client_status(sockaddr_in client);

// temporary functions
void add_user(int id);
void add_message_by_id(int id, string msg);
string get_message_by_id(int id);
int check_for_messages(int id);
int interprete_message(string msg);
void *server_terminal(void * arg);

#define dlog(format, args...) printf(format, ##args)

typedef struct {
    int socket;
    sockaddr_in client_addr;
}ClientData;

typedef enum {
    IDLE = 0,
    GET_ONLINE_USERS,
    SEND_MATCH_REQUEST,
    GET_MATCH_REQUEST,
    SEND_RESPONSE_TO_MATCH_REQUEST,
    GET_RESPONSE_TO_MATCH_REQUEST,
    RESPOND_MATCH_REQUEST,
    GET_MESSAGE_REQUEST,
    SEND_MESSAGE_REQUEST,
    DISCONNECT_REQUEST,
    OPPONENT_DISCONNECTED,
    TEST
}RequestTypes;

bool g_is_active = true;


int main () {    
    Server server;
    Users users;
    
    #if 0
    char *test = (char *)malloc(sizeof(char));
    test[6] = 4;
    free(test);
    free(test);
    char c = test[6];
    printf("test '%c'\n", c);
    #endif
    
    if (server.init_server() == -1)
        return -1;
    
    server.wait_clients();
    
    return 0;
}

/* TODO: serveri kapatmak için eklendi ama tam çalışmıyor düzeltilmesi lazım */
void *server_terminal(void * arg) {
    char input[64];
    
    for (;;) {
        memset(&input, 0, 64);
        
        scanf(" %[^\n]", input);
        
        if (strcmp(input, "close server") == 0) {
            g_is_active = false;
            break;
        }
    }
    
    return NULL;
}

