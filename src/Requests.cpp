#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "Requests.h"
#include "Users.h"
#include "Jwt.h"
#include "Protocol.h"
#include "utilities.h"
#include "Matcher.h"
#include "debug.h"

// socket
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

Requests::Requests(int sock) {
    socket = sock;
}

Requests::~Requests() {
    _out_packet.free_buffer();
    _in_packet.free_buffer();
}

void Requests::send_response() {
    _out_packet.send_packet(socket);
}

ErrorCodes Requests::get_request() {
    ErrorCodes err = ERR_SUCCESS;
    RequestCodes req_code;
    string indata;
    int uid;
    
    if (!_in_packet.receive_packet(socket)) {
        mlog.log_error("ERROR RECEIVE PACKET");
        err = ERR_CONNECTION;
        _in_packet.free_buffer();
        set_header(REQUEST_HEADER);
        return err;
    }
    
    // check request
    err = check_request(&uid);
    if (err != ERR_SUCCESS) {
        mlog.log_error("ERROR CODE: %d", err);
        _in_packet.free_buffer();
        set_header(REQUEST_HEADER);
        return err;
    }
    
    req_code = (RequestCodes)_in_packet.get_request_code();
    if (req_code != REQ_FB_LOGIN)
        indata = _in_packet.get_data().substr(TOKEN_SIZE, _in_packet.get_length() - TOKEN_SIZE);
    else
        indata = _in_packet.get_data();
    
    _in_packet.free_buffer(); // i wont use receiving buffer anymore, free.
    
    err = interpret_request(uid, req_code, indata);
    if (err != ERR_SUCCESS) {
        mlog.log_error("ERROR INTERPRET: %d", err);
        return err;
    }
    
    return err;
}

ErrorCodes Requests::handle_request() {
    ErrorCodes ret = get_request();
    if (ret == ERR_CONNECTION) {
        mlog.log_error("Client disconnected");
        logout(get_uid(socket));
        
        // TODO: if matched, send notification to opponent. maybe with event
        return ERR_REQ_DISCONNECTED;
    }
    else if (ret != ERR_SUCCESS) {
        prepare_error_packet(ret);
    }
    
    send_response();
    
    return ERR_SUCCESS;
}

ErrorCodes Requests::check_request(int *p_uid) {
    ErrorCodes ret = ERR_REQ_UNKNOWN;
    
    // check header
    if (_in_packet.get_header() != REQUEST_HEADER)
        return ERR_REQ_WRONG_HEADER;
    
    // check crc
    if (!_in_packet.check_crc())
        return ERR_REQ_CRC;
    
    // don't check token if login request
    if (_in_packet.get_request_code() == REQ_FB_LOGIN)
        return ERR_SUCCESS;
    
    if (!_in_packet.check_token(jwt_key, p_uid)) {
        return ERR_REQ_WRONG_TOKEN;
    }
    
    return ERR_SUCCESS;
}

ErrorCodes Requests::interpret_request(int uid, RequestCodes req_code, string indata) {
    Server *srv = Server::get_instance();
    Users *users = Users::get_instance();
    ErrorCodes ret = ERR_SUCCESS;
    
    mlog.log_debug("Request code: %d", req_code);
    
    set_header(REQUEST_HEADER);
    if (req_code == REQ_FB_LOGIN) {
      // no length check. fb token size may vary
#if 0
        // TODO: check length(NOT FINISHED)
        if (indata.size() = 0) {
            ret = ERR_REQ_WRONG_LENGTH;
            goto L_ERROR;
        }
#endif
        // TODO: test this request
        string fb_token = indata;
        
        ret = users->register_user(&uid, fb_token);
        // TODO: Add all facebook errors
        switch (ret) {
            case ERR_SUCCESS:
                mlog.log_debug("fb success");
                set_request_code(REQ_FB_LOGIN);
                set_token(uid);
                ClientConnectionInfo user_conn;
                user_conn.socket = socket;
                user_conn.uid = uid;
                
                login(user_conn);
                break;
            case ERR_FB_UNKNOWN:
            case ERR_FB_INVALID_ACCESS_TOKEN:
                mlog.log_debug("fb error");
                goto L_ERROR;
                break;
            default:
                mlog.log_debug("fb default");
                break;
        }
    }
    else if (req_code == REQ_LOGOUT) {
        if(indata.size() != 0) {
            ret = ERR_REQ_WRONG_LENGTH;
            goto L_ERROR;
        }
        
        logout(uid);
        
        set_request_code(REQ_LOGOUT);
        uint8_t outdata = ACK;
        add_data(&outdata, 1);
    }
    else if (req_code == REQ_GET_ONLINE_USERS) {
        // TODO: test this request
        if (indata.size() != 0) {
            mlog.log_debug("size: %d", (int)indata.size());
            mlog.log_hex((const char *)"indata:", (char *)indata.c_str(), indata.size());
            ret = ERR_REQ_WRONG_LENGTH;
            goto L_ERROR;
        }
        
        set_request_code(REQ_GET_ONLINE_USERS);
        
        // get online users
        vector<int> onl_cli = srv->get_online_clients();
        
        // add count
        uint8_t buffer[2] = {B0(onl_cli.size()), B1(onl_cli.size())};
        add_data(buffer, 2);
        
        // add client ids
        for (uint16_t i = 0; i < onl_cli.size(); i++) {
            uint8_t buffer[4] = {   
                                    B0(onl_cli[i]),
                                    B1(onl_cli[i]),
                                    B2(onl_cli[i]),
                                    B3(onl_cli[i])
                                };
            
            add_data(buffer, 4);
        }
    }
    else if (req_code == REQ_MATCH) {
        if (indata.size() != 0) {
            mlog.log_debug("size: %d", (int)indata.size());
            mlog.log_hex((const char *)"indata:", (char *)indata.c_str(), indata.size());
            ret = ERR_REQ_WRONG_LENGTH;
            goto L_ERROR;
        }
        UserMatchInfo user;
        mlog.log_debug("match request from : %d", uid);
        // find opponent
        // 1- add client's uid to the 'waiting matches list'
        //  Note: matcher will find an opponent from waiting list and throws an event
        user.uid = uid;
        user.op_uid = 0;
        add_to_match_queue(&user);
        
        // 2- wait for match with timeout
        mlog.log_debug("waiting match for %d...", uid);
        time_t start_dt = time(0);
        ret = is_matched(&user, start_dt, true);
        if (ret != ERR_SUCCESS) {
            // match failed. send error message
            mlog.log_debug("match has timed out");
            goto L_ERROR;
        }
        
        // create new game
        int game_id = 0;
        if (user.uid > user.op_uid) {
            Rivals riv;
            riv.user1.uid = user.uid;
            riv.user1.socket = socket;
            riv.user1.accept = false;
            riv.user2.uid = user.op_uid;
            riv.user2.socket = get_socket(user.op_uid);
            riv.user2.accept = false;
            game_id = create_game(riv);
            // is game id must be saved?
        }
        else {
            while (game_id == 0)
                game_id = get_game_id(user.uid);
        }
        
        // match success
        mlog.log_debug("%d matched with %d", user.uid, user.op_uid);
        set_request_code(REQ_MATCH);
        
        // 3- get opponent info :
        nlohmann::json op = users->get_user_data(user.op_uid);
        op["game_id"] = game_id;
        // 4- prepare package for transmitting to the client
        add_data(op.dump());
#if 0
        uint8_t buffer[4] = {
            B0(user.op_uid),
            B1(user.op_uid),
            B2(user.op_uid),
            B3(user.op_uid)
        };
        
        add_data(buffer, 4);
#endif
        // end
    }
    else if (req_code == REQ_CANCEL_MATCH) {
        if (indata.size() != 0) {
            mlog.log_debug("size: %d", (int)indata.size());
            mlog.log_hex((const char *)"indata:", (char *)indata.c_str(), indata.size());
            ret = ERR_REQ_WRONG_LENGTH;
            goto L_ERROR;
        }
        
        // remove user from match queue
        cancel_match(uid);
        
        set_request_code(REQ_CANCEL_MATCH);
        uint8_t data = ACK;
        add_data(&data, 1);
    }
    else if(req_code == REQ_START_GAME) {
        // get game id from indata
        
        Game *game = GameService::get_instance()->lookup(uid);
        
        if (game == NULL) {
            ret = ERR_GAME_NOT_MATCHED;
            goto L_ERROR;
        }
        
        // set acception for this client
        game->accept_game(uid);
        
        // wait for opponents answer
        while(!game->is_ready());
        
        // start game
        game->start_game(uid);
        
        // get questions
        string questions = game->get_questions();
        nlohmann::json data_json = nlohmann::json::parse(questions);
        data_json["start_dt"] = game->get_start_dt();
        set_request_code(REQ_START_GAME);
        
        // send questions
        add_data(data_json.dump());
    }
    else {
        // unknown request received
        ret = ERR_REQ_WRONG_REQ_CODE;
        goto L_ERROR;
    }
    
    _out_packet.set_crc();
    
    L_ERROR:
    return ret;
}

void Requests::prepare_error_packet(ErrorCodes err) {
    _out_packet.set_request_code(REQ_ERROR);
    uint8_t buffer = (uint8_t)(err & 0xFF);
    _out_packet.add_data(&buffer, 1);
    _out_packet.set_crc();
}

bool Requests::check_request_code() {
    if (_in_packet.get_request_code() < REQ_COUNT)
        return true;
    
    return false;
}

bool Requests::set_header(uint8_t header) {
    return _out_packet.set_header(header);
}

bool Requests::set_request_code(RequestCodes req_code) {
    return _out_packet.set_request_code((uint8_t)(req_code & 0xFF));
}

#if 0
bool Requests::set_ack(uint8_t ack) {
    return _out_packet.set_ack(ack);
}
#endif

bool Requests::set_token(int id) {
    time_t start_dt = time(NULL); // get current dt
    Jwt token(id, start_dt, jwt_key);
    
    return _out_packet.add_data(token.get_token());
}

bool Requests::add_data(string data) {
    _out_packet.add_data(data);
    return true;
}

bool Requests::add_data(uint8_t *data, uint16_t len) {
    return _out_packet.add_data(data, len);
}

void Requests::login(ClientConnectionInfo client_conn) {
    //login
    Server::get_instance()->login(client_conn);
}

void Requests::logout(int client_id) {
    //logout
    Server::get_instance()->logout(client_id);
}

void Requests::add_to_match_queue(UserMatchInfo *user) {
    Matcher::get_instance()->add(user);
}

void Requests::remove_from_match_queue(UserMatchInfo *user) {
    Matcher::get_instance()->remove(user);
}

void Requests::cancel_match(int uid) {
    UserMatchInfo *user = Matcher::get_instance()->lookup(uid);
    if (user != NULL)
        Matcher::get_instance()->remove(user);
}

ErrorCodes Requests::is_matched(UserMatchInfo *user, time_t start, bool is_blocking) {
    if (!is_blocking) {
        if (user->op_uid == 0)
            return ERR_REQ_MATCH_FAIL;
        
        return ERR_SUCCESS;
    }
    
    while(user->op_uid == 0) {
        // wait
        if (time(0) > (start + MATCH_TIMEOUT)) {
            remove_from_match_queue(user);
            return ERR_REQ_MATCH_FAIL;
        }
    }
    
    return ERR_SUCCESS;
}

int Requests::create_game(Rivals rivals) {
    return GameService::get_instance()->create_game(rivals);
}

int Requests::get_socket(int op_uid) {
    return Server::get_instance()->get_socket(op_uid);
}

int Requests::get_uid(int socket) {
    return Server::get_instance()->get_uid(socket);
}

int Requests::get_game_id(int uid) {
    return GameService::get_instance()->get_game_id(uid);
}
