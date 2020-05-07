#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "Requests.h"
#include "RegistryService.h"
#include "GameService.h"
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

#include <queue>
#include <deque>

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
    
    mlog.log_debug("notification count: %d", (int)_notifications.size());
    if (_notifications.size()) {
        mlog.log_debug("notifications sending... (%d)", (int)_notifications.size());
    }
    useconds_t usec = 500000;
    //usleep(usec);
    
    // send notifications that relevant with this request
    while (_notifications.size() != 0) {
        mlog.log_debug("1remaining notification: %d", _notifications.size());
        mlog.log_debug("notification socket: %d", _notifications.front().socket);
        _notifications.front().packet.send_packet(_notifications.front().socket);
        _notifications.pop();
        mlog.log_debug("2remaining notification: %d", _notifications.size());
        //usleep(usec);
    }
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
        mlog.log_error("Client disconnected (uid: %lu)", get_uid(socket));
        
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
            {
                set_request_code(REQ_FB_LOGIN);
                set_token(uid);
                ClientConnectionInfo user_conn = {uid, socket};
                login(user_conn);
                break;
            }
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
                                    B4(onl_cli[i]),
                                    B5(onl_cli[i]),
                                    B6(onl_cli[i]),
                                    B7(onl_cli[i]),
                                };
            
            add_data(buffer, 8);
        }
    }
    else if (req_code == REQ_MATCH) {
        if (indata.size() != 0) {
            mlog.log_error("size: %d", (int)indata.size());
            ret = ERR_REQ_WRONG_LENGTH;
            goto L_ERROR;
        }
        
        mlog.log_debug("match request from : %lu", uid);
        
        // find opponent
        // 1- add client's uid to the 'waiting matches list'
        //  Note: matcher will find an opponent from waiting list and throws an event
        UserMatchInfo *user = new UserMatchInfo();
        user->uid = uid;
        user->socket = socket;
        user->start_time = time(NULL);
        
        ret = add_to_match_queue(user);
        if (ret != ERR_SUCCESS) {
            // already added
            goto L_ERROR;
        }
        
        set_request_code(REQ_MATCH);
        // send ack
        uint8_t data = ACK;
        add_data(&data, 1);
    }
    else if (req_code == REQ_CANCEL_MATCH) {
        ret = check_game_request(REQ_CANCEL_MATCH, indata);
        if (ret != ERR_SUCCESS) {
            goto L_ERROR;
        }
        
        nlohmann::json data_json = nlohmann::json::parse(indata);
        int game_id = data_json["game_id"];
        Game *game = GameService::get_instance()->lookup(game_id);
        GameUser op = game->get_opponent(uid);
        
        // remove user from match queue
        cancel_match(uid, game_id);
        
        // TEST: send notification to opponent about cancellation
        add_notification(op.socket, REQ_GAME_NOT_ACCEPTED, "");
        
        set_request_code(REQ_CANCEL_MATCH);
        uint8_t data = ACK;
        add_data(&data, 1);
    }
    else if(req_code == REQ_GAME_START) {
        ret = check_game_request(REQ_GAME_START, indata);
        if (ret != ERR_SUCCESS) {
            goto L_ERROR;
        }
        
        nlohmann::json data_json = nlohmann::json::parse(indata);
        int game_id = data_json["game_id"];
        Game *game = game = GameService::get_instance()->lookup(game_id);
        // get game id from indata
        if (game == NULL) {
            ret = ERR_GAME_NOT_MATCHED;
            goto L_ERROR;
        }
        
        uint8_t category = data_json["category"];
        
        // set this users category
        game->set_category(uid, category);
        
        // set acception for this client
        if (game->accept_game(uid)) {
            game->stop_timer();
            
            game->start_game();
            
            GameUser op = game->get_opponent(uid);
            string questions = game->get_questions();
            
            add_notification(socket, REQ_GAME_ACCEPTED, questions);
            add_notification(op.socket, REQ_GAME_ACCEPTED, questions);
        }
        
        set_request_code(REQ_GAME_START);
        uint8_t data = ACK;
        add_data(&data, 1);
    }
    else if (req_code == REQ_GAME_ANSWER) {
        ret = check_game_request(REQ_GAME_ANSWER, indata);
        if (ret != ERR_SUCCESS) {
            // wrong packet
            mlog.log_error("check answer error: %d", ret);
            goto L_ERROR;
        }
        
        nlohmann::json data_json = nlohmann::json::parse(indata);
        int game_id = data_json["game_id"];
        
        // check if game exists
        Game *game = GameService::get_instance()->lookup(game_id);
        if (game == NULL) {
            ret = ERR_GAME_NOT_FOUND;
            mlog.log_error("game not found error");
            goto L_ERROR;
        }
        
        if (game->is_answered(uid)) {
            ret = ERR_GAME_ALREADY_ANSWERED;
            mlog.log_error("game already answered error");
            goto L_ERROR;
        }
        
        // TEST: set as answered this user
        game->set_answer(uid);
        
        GameUser op = game->get_opponent(uid);
        
        // add game answer request to the queue for the opponent user
        mlog.log_debug("op.socket: %d", op.socket);
        add_notification(op.socket, REQ_GAME_OPPONENT_ANSWER, indata);
        
        // if both of rivals has answered current question
        // send question complete notification both of them
        if (!game->check_timer()) {
            mlog.log_debug("both of rivals answered");
            
            // for opponent
            add_notification(op.socket, REQ_GAME_QUESTION_COMPLETED, "");
            
            // for this user
            add_notification(socket, REQ_GAME_QUESTION_COMPLETED, "");
            
            // start a timer to check if this question has timed out
            game->start_timer();
        }
        
        // send ack to this user
        set_request_code(REQ_GAME_ANSWER);
        uint8_t outdata = ACK;
        add_data(&outdata, 1);
    }
    else if (req_code == REQ_GAME_RESIGN) {
        ret = check_game_request(REQ_GAME_RESIGN, indata);
        if (ret != ERR_SUCCESS) {
            goto L_ERROR;
        }
        
        nlohmann::json data_json = nlohmann::json::parse(indata);
        
        int game_id = data_json["game_id"];
        Game *game = GameService::get_instance()->lookup(game_id);
        if (game == NULL) {
            ret = ERR_GAME_NOT_FOUND;
            goto L_ERROR;
        }
        
        // mark this user as resigned on game object
        game->resign(uid);
        
        GameService::get_instance()->remove_game(game_id);
        
        set_request_code(REQ_GAME_RESIGN);
        uint8_t outdata = ACK;
        add_data(&outdata, 1);
    }
    else if (req_code == REQ_GAME_FINISH) {
        ret = check_game_request(REQ_GAME_RESIGN, indata);
        if (ret != ERR_SUCCESS) {
            goto L_ERROR;
        }
        
        nlohmann::json data_json = nlohmann::json::parse(indata);
        int game_id = data_json["game_id"];
        string results = data_json["results"].dump();
        Game *game = GameService::get_instance()->lookup(game_id);
        // release game object and its content (timer etc.)
        if (game != NULL) {
            GameService::get_instance()->finish_game(game, results);
            mlog.log_debug("game finished");
        }
        
        set_request_code(REQ_GAME_FINISH);
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

void Requests::add_notification(int socket, RequestCodes req_code, string data) {
    NotificationInfo ni;
    ni.packet.set_header(REQUEST_HEADER);
    ni.packet.set_request_code(req_code);
    ni.packet.add_data(data);
    ni.packet.set_crc();
    ni.socket = socket;
    _notifications.push(ni);
}

void Requests::send_notification_async(int socket, RequestCodes req_code, string data) {
    Protocol packet;
    packet.set_header(REQUEST_HEADER);
    packet.set_request_code(req_code);
    packet.add_data(data);
    packet.set_crc();
    packet.send_packet(socket);
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
    Matcher::get_instance()->remove(uid);
    
    // remove game if exists
    // TODO: user must be resign. do not remove game, resign user.
    Game *game = GameService::get_instance()->lookup_by_uid(uid);
    if (game != NULL) {
        game->resign(uid);
    }
    
    //logout
    Server::get_instance()->logout(uid);
}

ErrorCodes Requests::add_to_match_queue(UserMatchInfo *user) {
    return Matcher::get_instance()->add(user);
}

void Requests::remove_from_match_queue(UserMatchInfo *user) {
    Matcher::get_instance()->remove(user);
}

void Requests::cancel_match(uint64_t uid, int game_id) {
    // remove match request
    Matcher::get_instance()->remove(uid);
    
    // remove created game
    GameService::get_instance()->remove_game(game_id);
}

Game *Requests::create_game(Rivals rivals) {
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

ErrorCodes Requests::check_game_request(RequestCodes req_code, string data) {
    bool ret = false;
    nlohmann::json answer_json;
    
    if (!nlohmann::json::accept(data))
        return ERR_GAME_WRONG_PACKET;
    
    answer_json = nlohmann::json::parse(data);
    switch (req_code) {
        case Requests::REQ_GAME_START:
            ret = answer_json.contains("game_id") && answer_json.contains("category");
            break;
        case Requests::REQ_GAME_ANSWER:
            ret = answer_json.contains("game_id") && answer_json.contains("answer");
            break;
        case Requests::REQ_CANCEL_MATCH:
        case Requests::REQ_GAME_RESIGN:
            ret = answer_json.contains("game_id");
            break;
        case Requests::REQ_GAME_FINISH:
            ret = answer_json.contains("game_id") && answer_json.contains("results");
            break;
    }
    
    if (!ret) return ERR_GAME_WRONG_PACKET;
    
    return ERR_SUCCESS;
}
