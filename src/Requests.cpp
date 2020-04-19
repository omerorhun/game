#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "Requests.h"
#include "RegistryService.h"
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
    uint64_t uid;
    
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

ErrorCodes Requests::check_request(uint64_t *p_uid) {
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

ErrorCodes Requests::interpret_request(uint64_t uid, RequestCodes req_code, string indata) {
    Server *srv = Server::get_instance();
    RegistryService *auth_service = RegistryService::get_instance();
    ErrorCodes ret = ERR_SUCCESS;
    
    mlog.log_debug("Request code: %d", req_code);
    
    set_header(REQUEST_HEADER);
    if (req_code == REQ_FB_LOGIN) {
      // no length check. fb token size may vary
        string fb_token = indata;
        
        ret = auth_service->sign_in(&uid, fb_token);
        // check if user logged in
        if (srv->is_client_online(uid)) {
            ret = ERR_LOGIN_ALREADY_LOGGED_IN;
            goto L_ERROR;
        }
        
        // TODO: Add all facebook errors
        switch (ret) {
            case ERR_SUCCESS:
                set_request_code(REQ_FB_LOGIN);
                set_token(uid);
                ClientConnectionInfo user_conn;
                user_conn.socket = socket;
                user_conn.uid = uid;
                
                login(user_conn);
                break;
            case ERR_FB_UNKNOWN:
            case ERR_FB_INVALID_ACCESS_TOKEN:
                mlog.log_error("fb error");
                goto L_ERROR;
                break;
            default:
                mlog.log_error("fb default");
                goto L_ERROR;
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
            ret = ERR_REQ_WRONG_LENGTH;
            goto L_ERROR;
        }
        
        set_request_code(REQ_GET_ONLINE_USERS);
        
        // get online users
        vector<uint64_t> onl_cli = srv->get_online_clients();
        
        // add count
        uint8_t buffer[2] = {B0(onl_cli.size()), B1(onl_cli.size())};
        add_data(buffer, 2);
        
        // add client ids
        for (uint16_t i = 0; i < onl_cli.size(); i++) {
            uint8_t buffer[8] = {   
                                    B0(onl_cli[i]),
                                    B1(onl_cli[i]),
                                    B2(onl_cli[i]),
                                    B3(onl_cli[i]),
                                    // TODO: convert here to 64 bit version
                                };
            
            add_data(buffer, 4);
        }
    }
    else if (req_code == REQ_MATCH) {
        if (indata.size() != 0) {
            mlog.log_error("size: %d", (int)indata.size());
            ret = ERR_REQ_WRONG_LENGTH;
            goto L_ERROR;
        }
        UserMatchInfo user;
        mlog.log_debug("match request from : %lu", uid);
        // find opponent
        // 1- add client's uid to the 'waiting matches list'
        //  Note: matcher will find an opponent from waiting list and throws an event
        user.uid = uid;
        user.op_uid = 0;
        add_to_match_queue(&user);
        
        // 2- wait for match with timeout
        mlog.log_debug("waiting match for %lu...", uid);
        time_t start_dt = time(0);
        ret = is_matched(&user, start_dt, true);
        if (ret != ERR_SUCCESS) {
            // match failed. send error message
            mlog.log_error("match has timed out");
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
        mlog.log_debug("%lu matched with %lu", user.uid, user.op_uid);
        set_request_code(REQ_MATCH);
        mlog.log_debug("debug");
        // 3- get opponent info :
        RegistryInfo op = auth_service->get_user_data(user.op_uid);
        mlog.log_debug("debug");
        nlohmann::json op_json;
        op_json["id"] = op.uid;
        op_json["fb_id"] = op.fb_id;
        op_json["name"] = op.name;
        op_json["url"] = op.picture_url;
        op_json["game_id"] = game_id;
        mlog.log_debug("debug");
        // 4- prepare package for transmitting to the client
        add_data(op_json.dump());
        mlog.log_debug("debug");
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
            mlog.log_error("size: %d", (int)indata.size());
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
        
        // TODO: add timeout!
        // wait for opponents answer
        time_t start_dt = time(NULL);
        ErrorCodes ret = game->is_ready(start_dt, true);
        if (ret != ERR_SUCCESS) {
            // match failed. send error message
            mlog.log_error("game start has timed out [error code: %d]", ret);
            goto L_ERROR;
        }
        
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
    else if (req_code == REQ_GAME_ANSWER) {
        // TODO: check if game exists
        Game *game = GameService::get_instance()->lookup(uid);
        if (game == NULL) {
            ret = ERR_GAME_NOT_FOUND;
            goto L_ERROR;
        }
        
        // check data and get answer
        string answer;
        ErrorCodes ret = get_game_answer(indata, answer);
        if (ret != ERR_SUCCESS) {
            // wrong packet
            goto L_ERROR;
        }
        
        GameUser op = game->get_opponent(uid);
        // add game answer request to the queue for the opponent user
        mlog.log_debug("op.socket: %d", op.socket);
        send_notification(op.socket, REQ_GAME_ANSWER, answer);
        
        // send ack to this user
        set_request_code(REQ_GAME_ANSWER);
        uint8_t outdata = ACK;
        add_data(&outdata, 1);
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

void Requests::send_notification(int socket, RequestCodes req_code, string data) {
    Protocol packet;
    packet.set_header(REQUEST_HEADER);
    packet.set_request_code(req_code);
    packet.add_data(data);
    packet.set_crc();
    packet.send_packet(socket);
}

ErrorCodes Requests::get_game_answer(string data, string &answer) {
    nlohmann::json answer_json;
    if (!nlohmann::json::accept(data)) {
        return ERR_GAME_WRONG_PACKET;
    }
    
    answer_json = nlohmann::json::parse(data);
    if (answer_json.find("answer") == answer_json.end()) {
        return ERR_GAME_WRONG_PACKET;
    }
    
    // TODO: not need to copy. can be used the 'data' object
    answer = data;
    
    return ERR_SUCCESS;
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

bool Requests::set_token(uint64_t uid) {
    time_t start_dt = time(NULL); // get current dt
    Jwt token(uid, start_dt, jwt_key);
    
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

void Requests::logout(uint64_t uid) {
    // if match request belongs to user exists..
    cancel_match(uid);
    
    // remove game if exists
    // TODO: user must be resign. do not remove game, resign user.
    //GameService::get_instance()->finish_game(uid);
    
    //logout
    Server::get_instance()->logout(uid);
}

void Requests::add_to_match_queue(UserMatchInfo *user) {
    Matcher::get_instance()->add(user);
}

void Requests::remove_from_match_queue(UserMatchInfo *user) {
    Matcher::get_instance()->remove(user);
}

void Requests::cancel_match(uint64_t uid) {
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

int Requests::get_socket(uint64_t op_uid) {
    return Server::get_instance()->get_socket(op_uid);
}

uint64_t Requests::get_uid(int socket) {
    return Server::get_instance()->get_uid(socket);
}

int Requests::get_game_id(uint64_t uid) {
    return GameService::get_instance()->get_game_id(uid);
}
