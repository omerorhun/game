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
//#include "ClientInfo.h"
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

//map<int, ClientInfo> online_clients;
bool g_is_active = true;

int main () {
    /*
    ClientInfo new_client;
    
    // Create a socket 
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        cerr << "Can't create a socket!";
        return -1;
    }
    
    // Bind the socket to an IP/port
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT_NO);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
    char lstn[NI_MAXHOST];
    memset(&lstn, 0, NI_MAXHOST);
    
    inet_ntop(AF_INET, &hint.sin_addr, lstn, NI_MAXHOST);
    
    cout << "Listening on " << string(lstn) << endl;
    
    if (bind(listening, (const sockaddr *)&hint, sizeof(hint)) == -1) {
        cerr << "Can't bind to IP/port" << endl;
        return -2;
    }
    
    // Mark the socket for listening in
    if (listen(listening, SOMAXCONN) == -1) {
        cerr << "Can't listen!" << endl;
        return -3;
    }
    
    // Accept a call
    sockaddr_in client;
    socklen_t client_size = sizeof(client);
    
    pthread_t term_t;
    pthread_create(&term_t, NULL, server_terminal, NULL);
    
    int client_socket;
    for (;;) {
        
        if (g_is_active == false)
            break;
        
        if ((client_socket = accept(listening, (sockaddr *)&client, &client_size)) == -1) {
            cerr << "Can't accept a call!" << endl;
            return -4;
        }
        
        ClientData *cd = (ClientData *)malloc(sizeof(ClientData));
        cd->socket = client_socket;
        cd->client_addr = client;
        
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, client_operations, (void *)cd);
    }
    
    // Close the listening socket
    close(listening);
    */
    
    Server server;
    Users users;
    
    if (server.init_server() != -1) {
        server.wait_clients();
    }
    
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
/*
void *client_operations(void * arg) {
    ClientData cd = *((ClientData *)arg);
    free(arg);
    int client_socket = cd.socket;
    RequestTypes state = IDLE;
    
    print_client_status(cd.client_addr);
    
    // Register client
    ClientInfo new_client(client_socket);
    online_clients.insert(pair<int, ClientInfo>(new_client.get_id(), new_client));
    add_user(new_client.get_id());
    cout << "New Client (id: " << new_client.get_id() << ")" << endl;
    
    // create new thread for receiving messages from client
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, listen_user, (void *)&new_client);
    
    for (;;) {
        char request[BUFFER_SIZE];
        memset(&request, 0, BUFFER_SIZE);
        
        // check if new message from client
        // check if new message from opponent
        if (check_for_messages(new_client.get_id())) {
            string msg = get_message_by_id(new_client.get_id());
            // TODO: burada kaldi 25.02.2020 03:55
            
            cout << "new message is (string): " << msg << endl;
            sprintf(request, "%s", msg.c_str());
            state = (RequestTypes)interprete_message(msg);
            
            cout << "\nid" << new_client.get_id() << ": ";
            cout << "new message is: \""<< string(request) <<"\"" << endl;
            cout << "id" << new_client.get_id() << ": ";
            cout << "state: " << state << endl;
        }
        
        if (state == GET_ONLINE_USERS) {
            
            if (strcmp(request, "get_online") == 0) {
                char tx_buffer[BUFFER_SIZE];
                memset(&tx_buffer, 0, BUFFER_SIZE);
                sprintf(tx_buffer, "clients=");
                
                char *ptr = strstr(tx_buffer, "=");
                ptr++;
                
                for (int i = 0; i < online_clients.size(); i++) {
                    char temp[32];
                    memset(temp, 0, 32);
                    
                    ClientInfo ci = next(online_clients.begin(), i)->second;
                    sprintf(temp, "%d,", ci.get_id());
                    printf("ci.get_id(): %d\n", ci.get_id());
                    
                    printf("temp id: %s\n", temp);
                    strcat(tx_buffer, temp);
                }
                
                int len = strlen(tx_buffer);
                tx_buffer[len++-1] = '\r';
                tx_buffer[len++] = '\n';
                
                send(new_client.get_socket(), tx_buffer, strlen(tx_buffer), 0);
            }
            
            state = IDLE;
        }
        else if (state == SEND_MATCH_REQUEST) {
            int op_id;
            char *ptr = strstr(request, "id");
            
            if (ptr) {
                sscanf(ptr, "id=%d\n", &op_id);
                
                // check if the opponent is online
                if (online_clients.find(op_id) == online_clients.end()) {
                    // send error message to client
                    char error_msg[] = "Client is offline\r\n";
                    send(new_client.get_socket(), error_msg, strlen(error_msg), 0);
                    break;
                }
                
                // add the request to the opponents queue
                char buffer[64];
                memset(&buffer, 0, 64);
                sprintf(buffer, "match_get,id=%d\r\n", new_client.get_id());
                string msg(buffer);
                
                add_message_by_id(op_id, msg);
                printf("id:%d - message added to %d\n", new_client.get_id(), op_id);
            }
            
            state = IDLE;
        }
        else if (state == GET_MATCH_REQUEST) {
            // transmit the match request to client
            
            printf("get match request: %s\n", request);
            
            char *ptr = strstr(request, "id=");
            int request_owner_id = 0;
            sscanf(ptr, "id=%d", &request_owner_id);
            
            char tx_buffer[BUFFER_SIZE];
            memset(&tx_buffer, 0, BUFFER_SIZE);
            sprintf(tx_buffer, "Do you want to accept the match (id:%d)\r\n", request_owner_id);
            
            send(new_client.get_socket(), tx_buffer, strlen(tx_buffer), 0);
            
            state = IDLE;
        }
        else if (state == SEND_RESPONSE_TO_MATCH_REQUEST) {
            // add the response to the opponents queue
            char *ptr = strstr(request, "id=");
            
            int request_owner_id;
            sscanf(ptr, "id=%d\r\n", &request_owner_id);
            
            
            char buffer[64];
            memset(&buffer, 0, 64);
            if (strstr(ptr, "yes")) {
                sprintf(buffer, "accept,id=%d,yes\r\n", new_client.get_id());
                
                string msg(buffer);
                add_message_by_id(request_owner_id, msg);
                new_client.set_opponent_id(request_owner_id);
            }
            else {
                sprintf(buffer, "accept,id=%d,no\r\n", new_client.get_id());
                
                string msg(buffer);
                add_message_by_id(request_owner_id, msg);
            }
            
            state = IDLE;
        }
        else if (state == GET_RESPONSE_TO_MATCH_REQUEST) {
            // transmit the response to client
            char *ptr = strstr(request, "id=");
            int op_id;
            sscanf(ptr, "id=%d,yes\r\n", &op_id);
            
            ptr = strstr(ptr, ",");
            ptr++;
            if (strcmp(ptr, "yes\r\n") == 0) {
                new_client.set_opponent_id(op_id);
                
                // send message to know match has succeed
                char tx_buffer[] = "Matched\r\n";
                send(new_client.get_socket(), tx_buffer, strlen(tx_buffer), 0);
            }
            else {
                char error_msg[] = "Couldn't matched\r\n";
                send(new_client.get_socket(), error_msg, strlen(error_msg), 0);
            }
            
            // note: client can send messages to opponent from now
            state = IDLE;
        }
        else if (state == SEND_MESSAGE_REQUEST) {
            
            // TEST: check if opponent is online
            if (online_clients.find(new_client.get_opponent_id()) == online_clients.end()) {
                string msg = "disc_op";
                add_message_by_id(new_client.get_id(), msg);
            }
            else {
                // add the message to the opponents queue
                char *ptr = strstr(request, ",");
                if (ptr) {
                    ptr++;
                    string msg = "tx_msg,";
                    msg.append(string(ptr));
                    add_message_by_id(new_client.get_opponent_id(), msg);
                }
            }
            
            state = IDLE;
        }
        else if (state == GET_MESSAGE_REQUEST) {
            // transmit the message to client
            char tx_buffer[1024];
            memset(&tx_buffer, 0, BUFFER_SIZE);
            sscanf(request, "tx_msg,%[^\n]\r\n", tx_buffer);
            send(new_client.get_socket(), tx_buffer, strlen(tx_buffer), 0);
            state = IDLE;
        }
        else if (state == DISCONNECT_REQUEST) {
            // if the request is a disconnection request
            // let the opponent know that client is disconnected (add opponents queue)
            string msg = "disc_op\r\n";
            
            // if client matched, send opponent notification
            if (new_client.get_opponent_id() != -1)
                add_message_by_id(new_client.get_opponent_id(), msg);
                
            // close socket
            close(new_client.get_socket());
            state = IDLE;
        }
        else if (state == OPPONENT_DISCONNECTED) {
            // let the client know that opponent is disconnected (transmit to client)
            char tx_buffer[] = "Client disconnected\r\n";
            send(new_client.get_socket(), tx_buffer, strlen(tx_buffer), 0);
            
            // close socket
            //close(new_client.get_socket());
            state = IDLE;
        }
        else if (state == TEST) {
            
        }
        
        state = IDLE;
    }
    
    return NULL;
}

void *listen_user(void *arg) {
    cout << "new listen" << endl;
    ClientInfo ci = *((ClientInfo *)arg);
    int socket = ci.get_socket();
    int c_id = ci.get_id();
    char buffer[BUFFER_SIZE];
    
    for (;;) {
        // clear buffer
        memset(&buffer, 0, BUFFER_SIZE);
        
        // wait for new message
        int rx_bytes = recv(socket, buffer, BUFFER_SIZE, 0);
        send(socket, "ack\r\n", 5, 0);
        
        cout << "id" << ci.get_id() << ": ";
        cout << "Client: " << string(buffer) << endl;
        
        if ((strcmp(buffer, "quit") == 0) || (rx_bytes == 0)) {
            cout << "id" << ci.get_id() << ": ";
            cout << "Client disconnected" << endl;
            string msg = "disc_cli\r\n";
            add_message_by_id(ci.get_id(), msg);
            break;
        }
        
        if (rx_bytes != -1) {
            // add message to corresponding queue and increase corresponding count
            add_message_by_id(ci.get_id(), string(buffer));
        }
    }
    
    return NULL;
}

int find_opponent(int id) {
    // Find an opponent
    // Send invitation to opponent
}

map <int, vector<string> > queue;

void add_user(int id) {
    vector<string> subqueue;
    int size = queue.size();
    queue.insert(pair<int,vector<string> >(id, subqueue));
    
    if (size == queue.size())
        cout << "error on adding user\n";
    
}

void remove_user(int id) {
    // TODO: maybe we must erase vector's content first?
    queue.erase(id);
}

void add_message_by_id(int id, string msg) {
    queue.at(id).push_back(msg);
}

string get_message_by_id(int id) {
    vector<string> *p_user = &queue.at(id);
    
    if (queue.at(id).size() > 0) {
        string msg = (*p_user).front();
        p_user->erase(p_user->begin());
        return msg;
    }
    
    return string("Empty");
}

#include <mutex>

mutex mtx;

int check_for_messages(int id) {
    //cout << id <<  "line: " << __LINE__ << endl;
    int size = 0;
    
    try {
        mtx.lock();
        size = queue.at(id).size();
        mtx.unlock();
        
        if (size > 0)
            printf("%d messages for user%d\n", size, id);
    }
    catch (const out_of_range& oor){
        cout << "queue size: " << queue.size() << endl;
        cout << "out of range " << oor.what() << endl;
    }
    
    return size;
}

int interprete_message(string msg) {
    int state = IDLE;
    
    if (msg.find("message_send") != string::npos) {
        state = SEND_MESSAGE_REQUEST;
    }
    else if (msg.find("tx_msg") != string::npos) {
        state = GET_MESSAGE_REQUEST;
    }
    else if(msg.find("get_online") != string::npos) {
        state = GET_ONLINE_USERS;
    }
    else if (msg.find("match_send") != string::npos) {
        state = SEND_MATCH_REQUEST;
    }
    else if (msg.find("match_get") != string::npos) {
        state = GET_MATCH_REQUEST;
    }
    else if (msg.find("match_response") != string::npos) {
        state = SEND_RESPONSE_TO_MATCH_REQUEST;
    }
    else if (msg.find("accept") != string::npos) {
        state = GET_RESPONSE_TO_MATCH_REQUEST;
    }
    else if (msg.find("disc_cli") != string::npos) {
        state = DISCONNECT_REQUEST;
    }
    else if (msg.find("disc_op") != string::npos) {
        state = OPPONENT_DISCONNECTED;
    }
    
    return state;
}

void print_client_status(sockaddr_in client) {
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    
    memset(&host, 0, NI_MAXHOST);
    memset(&svc, 0, NI_MAXSERV);
    
    int result = getnameinfo((const sockaddr *)&client, sizeof(client), 
                                            host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    
    if (result) {
        cout << string(host) << " connected to " << string(svc) << endl;
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << string(host) << " connected to " << ntohs(client.sin_port) << endl;
    }
    
    return;
}

*/


