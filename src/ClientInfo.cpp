#include "ClientInfo.h"
#include "Server.h"
#include "Users.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>

#include <iostream>
#include <stdio.h>
#include <string.h>


#define ACCESS_TOKEN_SIZE 128

int ClientInfo::client_count = 0;

ClientInfo::ClientInfo() {
    id = client_count++;
    availability = true;
}

ClientInfo::ClientInfo(int sock) {
    id = 1000 + client_count++;
    socket = sock;
    availability = true;
    opponent_id = -1;
    username = "";
    password = -1;
}

bool ClientInfo::is_available() {
    return availability;
}

void ClientInfo::set_socket(int sock) {
    socket = sock;
}

int ClientInfo::get_socket() {
    return socket;
}

void ClientInfo::set_id(int new_id) {
    id = new_id;
}

int ClientInfo::get_id() {
    return id;
}

void ClientInfo::set_session_id(int sid) {
    session_id = sid;
}

int ClientInfo::get_session_id() {
    return session_id;
}

void ClientInfo::set_username(string name) {
    username = name;
}

string ClientInfo::get_username(void) {
    return username;
}

void ClientInfo::set_opponent_id(int id) {
    opponent_id = id;
}

int ClientInfo::get_opponent_id(void) {
    return opponent_id;
}

void ClientInfo::set_fb_id(long fid) {
    fb_id = fid;
}

long ClientInfo::get_fb_id() {
    return fb_id;
}

void ClientInfo::set_picture_url(string url) {
    picture_url = url;
}

string ClientInfo::get_picture_url() {
    return picture_url;
}

int ClientInfo::listen(void) {
    thread ti(&ClientInfo::listen_user, this);
    
    message_handler();
}

void ClientInfo::listen_user(void) {
    char rx_buffer[RX_BUFFER_SIZE];
    
    for (;;) {
        memset(&rx_buffer, 0, RX_BUFFER_SIZE);
        int size = recv(socket, rx_buffer, RX_BUFFER_SIZE, 0);
        
        if (size == 0) {
            // add message to the queue
            Server::get_instance()->add_message_by_id(this->session_id, string("disc_cli"));
            break;
        }
        
        if (size != -1) {
            // add message to the queue
            Server::get_instance()->add_message_by_id(this->session_id, string(rx_buffer));
            
            //send(socket, "ack\r\n", 5, 0);
        }
    }
    
    return (void)NULL;
}

void ClientInfo::message_handler(void) {
    char request[RX_BUFFER_SIZE];
    RequestTypes state = IDLE;
    Server *srv = Server::get_instance();
    
    for (;;) {
        memset(&request, 0, RX_BUFFER_SIZE);
        
        // check if new message from client
        // check if new message from opponent
        if (srv->check_for_messages(session_id)) {
            string msg = srv->get_message_by_id(session_id);
            // TODO: burada kaldi 25.02.2020 03:55
            
            cout << "new message is (string): " << msg << endl;
            sprintf(request, "%s", msg.c_str());
            state = (RequestTypes)interprete_message(msg);
            
            cout << "session_id" << session_id << ": ";
            cout << "state: " << state << endl;
        }
        
        if (state == GET_ONLINE_USERS) {
            
            if (strcmp(request, "get_online") == 0) {
                char tx_buffer[RX_BUFFER_SIZE];
                memset(&tx_buffer, 0, RX_BUFFER_SIZE);
                sprintf(tx_buffer, "clients=");
                
                char *ptr = strstr(tx_buffer, "=");
                ptr++;
                
                vector<int> onl_cli = srv->get_online_clients();
                for (int i = 0; i < onl_cli.size(); i++) {
                    char temp[32];
                    memset(temp, 0, 32);
                    printf("%d,", onl_cli.at(i));
                    // TODO: burada kaldı. online kullanıcı bilgilerini gönder
                    
                    
                    
                    sprintf(temp, "%d,", onl_cli.at(i));
                    strcat(tx_buffer, temp);
                }
                
                int len = strlen(tx_buffer);
                tx_buffer[len++-1] = '\r';
                tx_buffer[len++] = '\n';
                
                printf("tx_buffer: %s-\n", tx_buffer);
                
                send(socket, tx_buffer, strlen(tx_buffer), 0);
            }
            
            state = IDLE;
        }
        else if (state == SEND_MATCH_REQUEST) {
            int op_id;
            char *ptr = strstr(request, "session_id");
            
            if (ptr) {
                sscanf(ptr, "session_id=%d\n", &op_id);
                
                // check if the opponent is online
                if (srv->is_client_online(op_id)) {
                    // send error message to client
                    char error_msg[] = "Client is offline\r\n";
                    send(socket, error_msg, strlen(error_msg), 0);
                    break;
                }
                
                // add the request to the opponents queue
                char buffer[64];
                memset(&buffer, 0, 64);
                sprintf(buffer, "match_get,session_id=%d\r\n", session_id);
                string msg(buffer);
                
                srv->add_message_by_id(op_id, msg);
                printf("session_id:%d - message added to %d\n", session_id, op_id);
            }
            
            state = IDLE;
        }
        else if (state == GET_MATCH_REQUEST) {
            // transmit the match request to client
            
            printf("get match request: %s\n", request);
            
            char *ptr = strstr(request, "session_id=");
            int request_owner_id = 0;
            sscanf(ptr, "session_id=%d", &request_owner_id);
            
            char tx_buffer[RX_BUFFER_SIZE];
            memset(&tx_buffer, 0, RX_BUFFER_SIZE);
            sprintf(tx_buffer, "Do you want to accept the match (session_id:%d)\r\n", request_owner_id);
            
            send(socket, tx_buffer, strlen(tx_buffer), 0);
            
            state = IDLE;
        }
        else if (state == SEND_RESPONSE_TO_MATCH_REQUEST) {
            // add the response to the opponents queue
            char *ptr = strstr(request, "session_id=");
            
            int request_owner_id;
            sscanf(ptr, "session_id=%d\r\n", &request_owner_id);
            
            char buffer[64];
            memset(&buffer, 0, 64);
            if (strstr(ptr, "yes")) {
                sprintf(buffer, "accept,session_id=%d,yes\r\n", session_id);
                
                string msg(buffer);
                srv->add_message_by_id(request_owner_id, msg);
                opponent_id = request_owner_id;
            }
            else {
                sprintf(buffer, "accept,session_id=%d,no\r\n", session_id);
                
                string msg(buffer);
                srv->add_message_by_id(request_owner_id, msg);
            }
            
            state = IDLE;
        }
        else if (state == GET_RESPONSE_TO_MATCH_REQUEST) {
            // transmit the response to client
            char *ptr = strstr(request, "session_id=");
            int op_id;
            sscanf(ptr, "session_id=%d,yes\r\n", &op_id);
            
            ptr = strstr(ptr, ",");
            ptr++;
            if (strcmp(ptr, "yes\r\n") == 0) {
                opponent_id = op_id;
                
                // send message to know match has succeed
                char tx_buffer[] = "Matched\r\n";
                send(socket, tx_buffer, strlen(tx_buffer), 0);
            }
            else {
                char error_msg[] = "Couldn't matched\r\n";
                send(socket, error_msg, strlen(error_msg), 0);
            }
            
            // note: client can send messages to opponent from now
            state = IDLE;
        }
        else if (state == SEND_MESSAGE_REQUEST) {
            
            // TEST: check if opponent is online
            if (srv->is_client_online(opponent_id)) {
                string msg = "disc_op";
                srv->add_message_by_id(session_id, msg);
            }
            else {
                // add the message to the opponents queue
                char *ptr = strstr(request, ",");
                if (ptr) {
                    ptr++;
                    string msg = "tx_msg,";
                    msg.append(string(ptr));
                    srv->add_message_by_id(opponent_id, msg);
                }
            }
            
            state = IDLE;
        }
        else if (state == GET_MESSAGE_REQUEST) {
            // transmit the message to client
            char tx_buffer[RX_BUFFER_SIZE];
            memset(&tx_buffer, 0, RX_BUFFER_SIZE);
            sscanf(request, "tx_msg,%[^\n]\r\n", tx_buffer);
            send(socket, tx_buffer, strlen(tx_buffer), 0);
            state = IDLE;
        }
        else if (state == DISCONNECT_REQUEST) {
            // if the request is a disconnection request
            // let the opponent know that client is disconnected (add opponents queue)
            string msg = "disc_op\r\n";
            logout();
            
            srv->add_message_by_id(opponent_id, msg);
            
            // close socket
            close(socket);
            
            state = IDLE;
        }
        else if (state == OPPONENT_DISCONNECTED) {
            // let the client know that opponent is disconnected (transmit to client)
            char tx_buffer[] = "Client disconnected\r\n";
            send(socket, tx_buffer, strlen(tx_buffer), 0);
            
            // close socket
            //close(new_client.get_socket());
            state = IDLE;
        }
        else if (state == SIGN_IN_REQUEST_RECEIVED) {
            char *ptr = strstr(request, ",");
            ptr++;
            
            char access_token[ACCESS_TOKEN_SIZE];
            if (ptr) {
                int ret = 0;
                if ((ret = sscanf(ptr, "access_token=%[^\n]\n", access_token))) {
                    cout << "ret: " << ret << endl;
                    cout << "access_token: " << string(access_token) << endl;
                    
                    char tx_buffer[RX_BUFFER_SIZE];
                    memset(&tx_buffer, 0, RX_BUFFER_SIZE);
                    ErrorUsers ret = Users::get_instance()->register_user(this, string(access_token));
                    switch (ret) {
                        case ERR_USERS_SIGNUP_SUCCESS:
                            login();
                            strcpy(tx_buffer, "Signed up\r\n");
                            break;
                        case ERR_USERS_LOGIN_SUCCESS:
                            login();
                            strcpy(tx_buffer, "Logged in\r\n");
                            break;
                        case ERR_USERS_FB:
                            strcpy(tx_buffer, "Error\r\n");
                            break;
                    }
                    
                    send(socket, tx_buffer, strlen(tx_buffer), 0);
                }
            }
            
            state = IDLE;
        }
        else if (state == LOGOUT_REQUEST) {
            logout();
        }
        else if (state == TEST) {
            
        }
        
        state = IDLE;
    }
    
    return;
}

int ClientInfo::interprete_message(string msg) {
    int state = IDLE;
    
    // TODO: messages must be declared as macros
    
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
    else if (msg.find("sign_in") != string::npos) {
        state = SIGN_IN_REQUEST_RECEIVED;
    }
    
    return state;
}

void ClientInfo::login() {
    Server::get_instance()->login(id);
}

void ClientInfo::logout() {
    Server::get_instance()->logout(id);
}
