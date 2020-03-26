#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "Requests.h"
#include "Server.h"
#include "Users.h"
#include "Jwt.h"
#include "Protocol.h"

// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

Requests::Requests(int sock) {
    socket = sock;
}

void Requests::send_response() {
    _out_packet.send_packet(socket);
}

ErrorInfo Requests::get_request(RequestCodes *p_req_code, string *p_indata) {
    ErrorInfo err = {ERR_SUCCESS, ERR_SUB_SUCCESS};
    
    if (!_in_packet.receive_packet(socket)) {
        cerr << "ERROR RECEIVE PACKET" << endl;
        err.code = ERR_REQ_CONNECTION;
        return err;
    }
    
    // check request
    ErrorCodes ret = check_request();
    if (ret != ERR_REQ_SUCCESS) {
        cerr << "ERROR CODE: " << ret << endl;
        return ret;
    }
    
    *p_req_code = (RequestCodes)_in_packet.get_request_code();
    *p_indata = _in_packet.get_data();
    
    _in_packet.free_buffer(); // i wont use receiving buffer from now, free.
    
    return ERR_REQ_SUCCESS;
}

void Requests::handle_request() {
    RequestCodes req_code;
    string indata;
    
    ErrorInfo ret = get_request(&req_code, &indata);
    if (ret.code == ERR_REQ_SUCCESS) {
        ret = interpret_request(req_code, indata);
    }
    
    if (ret.code != ERR_REQ_SUCCESS) {
        prepare_error_packet(ret);
    }
    
    send_response();
}

ErrorCodes Requests::check_request() {
    ErrorCodes ret = ERR_REQ_UNKNOWN;
    
    // check header
    if (_in_packet.get_header() != REQUEST_HEADER)
        return ERR_REQ_WRONG_HEADER;
    
    // check crc
    if (!_in_packet.check_crc())
        return ERR_REQ_CRC;
    
    if (!_in_packet.check_token(jwt_key))
        return ERR_REQ_WRONG_TOKEN;
    
    return ERR_REQ_SUCCESS;
}

bool Requests::check_request_code() {
    if (_in_packet.get_request_code() < REQ_COUNT)
        return true;
    
    return false;
}

ErrorInfo Requests::interpret_request(RequestCodes req_code, string indata) {
    Server *srv = Server::get_instance();
    Users *users = Users::get_instance();
    ErrorInfo ret = {ERR_REQ_SUCCESS, ERR_SUB_SUCCESS};
    
    cout << "Request code: " << req_code << endl;
    
    _out_packet.set_header(REQUEST_HEADER);
    if (req_code == REQ_FB_LOGIN) {
        // TODO: test this request
        string fb_token = indata;
        
        int client_id;
        ret = users->register_user(&client_id, fb_token);
        switch (ret.user) {
            case ERR_USERS_SIGNUP_SUCCESS:
            case ERR_USERS_LOGIN_SUCCESS:
            {
                _out_packet.set_request_code(REQ_FB_LOGIN);
                
                uint8_t ack = ACK;
                _out_packet.add_data(&ack, 1);
                
                time_t start_dt = time(NULL); // get current dt
                Jwt token(client_id, start_dt, jwt_key);
                _out_packet.add_data(token.get_token());
                
                login(client_id);
                break;
            }
            case ERR_USERS_FB:
                _out_packet.set_request_code(REQ_FB_LOGIN);
                uint8_t ack = NACK;
                _out_packet.add_data(&ack, 1);
                // TODO: add error code
                ret.code = ERR_REQ_LOGIN;
                break;
        }
    }
    else if (req_code == REQ_GET_ONLINE_USERS) {
        // TODO: test this request
        if (indata.size() == 0) {
            uint8_t nack = NACK;
            _out_packet.add_data(&nack, 1);
            prepare_error_packet(ERR_REQ_WRONG_LENGTH);
            
            
            return;
        }
        
        _out_packet.set_request_code(REQ_GET_ONLINE_USERS);
        uint8_t ack = ACK;
        _out_packet.add_data(&ack, 1);
        // get online users
        vector<int> onl_cli = srv->get_online_clients();
        
        // add count
        uint8_t buffer[2];
        buffer[0] = B0(onl_cli.size());
        buffer[1] = B1(onl_cli.size());
        _out_packet.add_data(buffer, 2);
        
        // add client ids
        for (uint16_t i = 0; i < onl_cli.size(); i++) {
            uint8_t buffer[2];
            buffer[0] = B0(onl_cli[i]);
            buffer[1] = B1(onl_cli[i]);
            buffer[2] = B2(onl_cli[i]);
            buffer[3] = B3(onl_cli[i]);
            _out_packet.add_data(buffer, 4);
        }
        
        
    }
    else {
        // unknown request received
        prepare_error_packet(ERR_REQ_WRONG_REQ_CODE);
    }
    
    _out_packet.set_crc();
    
    return;
}

void Requests::prepare_error_packet(ErrorCodes err) {
    _out_packet.set_request_code(REQ_ERROR);
    string errcode = to_string(err);
    _out_packet.add_data(errcode);
    _out_packet.set_crc();
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


void Requests::login(int client_id) {
    //login
    Server::get_instance()->login(client_id);
}

