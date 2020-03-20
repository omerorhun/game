#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "Requests.h"
#include "Server.h"
#include "Users.h"

// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

Requests::Requests(int sock) {
    socket = sock;
}

void Requests::get_request() {
    if (recv(socket, rx_buffer, RX_BUFFER_SIZE, 0) == -1) {
        cerr << "ERROR GET REQUEST" << endl;
        return;
    }
    
    // check request
    RequestErrorCodes ret = check_request();
    if (ret != ERR_REQ_SUCCESS) {
        cerr << "ERROR CODE: " << ret << endl;
        return;
    }
    
    length = GET_LENGTH(rx_buffer);
    req_code = GET_REQUEST_CODE(rx_buffer);
    
    // handle request
    handle_request();
    
    // send response
}

void Requests::handle_request() {
    char *request = this->rx_buffer;
    Server *srv = Server::get_instance();
    Users *users = Users::get_instance();
    
    cout << "Request code: " << this->req_code << endl;
    
    if (req_code == REQ_FB_LOGIN) {
        strncpy(token, (const char *)&rx_buffer[4], 128);
        int client_id;
        ErrorUsers ret = users->register_user(&client_id, string(token));
        switch (ret) {
            case ERR_USERS_SIGNUP_SUCCESS:
                login(client_id);
                strcpy(tx_buffer, "Signed up\r\n");
                break;
            case ERR_USERS_LOGIN_SUCCESS:
                login(client_id);
                strcpy(tx_buffer, "Logged in\r\n");
                break;
            case ERR_USERS_FB:
                strcpy(tx_buffer, "Error\r\n");
                break;
        }
        
    }
#if 0
    else if (req_code == REQ_GET_ONLINE_USERS) {
        data = (char *)malloc(sizeof(char) * )
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
        
        send(socket, tx_buffer, strlen(tx_buffer), 0);
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
    #endif
    

    return;

}

RequestErrorCodes Requests::check_request() {
    RequestErrorCodes ret = ERR_REQ_UNKNOWN;
    char *req = &rx_buffer[0];
    
    // check header
    if (rx_buffer[0] != REQUEST_HEADER) {
        return ERR_REQ_WRONG_HEADER;
    }
    
    // TODO: check request code
    if (!check_request_code()) {
        return ERR_REQ_WRONG_REQ_CODE;
    }
    
    // TODO: check length
    if (!check_length()) {
        return ERR_REQ_WRONG_LENGTH;
    }
    
    // TODO: check crc
    
    return ERR_REQ_SUCCESS;
}

bool Requests::check_length() {
    for (int i = 0; i < REQ_COUNT; i++) {
        if ((lengths[i][0] == GET_REQUEST_CODE(rx_buffer)) && 
                (lengths[i][1] == GET_LENGTH(rx_buffer))) {
            return true;
        }
    }
    
    return false;
}

bool Requests::check_request_code() {
    for (int i = 0; i < REQ_COUNT; i++) {
        if (rx_buffer[3] == i) return true;
    }
    
    return false;
}

void Requests::login(int client_id) {
    //login
    Server::get_instance()->login(client_id);
    Server::get_instance()->create_session(client_id);
}

