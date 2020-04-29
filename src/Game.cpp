#include <stdio.h>
#include <string>

#include "Game.h"
#include "Questions.h"
#include "Requests.h"
#include "debug.h"
#include "json.hpp"

using namespace std;

#define GAME_TIMEOUT 30

Game::Game(int game_id, Rivals rivals) {
    _game_uid = game_id;
    _rivals.user1 = rivals.user1;
    _rivals.user2 = rivals.user2;
    _state = GAME_WAITING_FOR_ACCEPTIONS;
    _current_tour = 0;
}

int Game::get_game_id() {
    return _game_uid;
}

Rivals Game::get_rivals() {
    return _rivals;
}

GameUser Game::get_opponent(uint64_t uid) {
    if (_rivals.user1.uid == uid) {
        return _rivals.user2;
    }
    
    return _rivals.user1;
}

bool Game::accept_game(uint64_t uid) {
    if (_rivals.user1.uid == uid) {
        _rivals.user1.accept = true;
        _rivals.user1.is_answered= true;
    }
    else if (_rivals.user2.uid == uid) {
        _rivals.user2.accept = true;
        _rivals.user2.is_answered= true;
    }
    
    if (_rivals.user1.accept && _rivals.user2.accept) {
        _state = GAME_RIVALS_READY;
        return true;
    }
    
    return false;
}

void Game::start_game() {
    // get questions from db
    string tour1 = Questions::get_instance()->get_question(5, _rivals.user1.category);
    string tour2 = Questions::get_instance()->get_question(5, _rivals.user2.category);
    string tour3 = Questions::get_instance()->get_question(5, 0);
    
    nlohmann::json temp;
    temp["tour1"].push_back(tour1);
    temp["tour2"].push_back(tour2);
    temp["tour3"].push_back(tour3);
    
    _questions = temp.dump();
    
    _state = GAME_QUESTIONS_READY;
    
    set_timer();
    start_timer();
}

string Game::get_questions() {
    return _questions;
}

void Game::set_category(uint64_t uid, uint8_t category) {
    if (_rivals.user1.uid == uid) {
        _rivals.user1.category = category;
    }
    else {
        _rivals.user2.category = category;
    }
}

void Game::timeout_func() {
    // mark user as 'timed out'
    mlog.log_debug("timeout func");
    
    if (!_rivals.user1.is_answered) {
        _rivals.user1.is_resigned = true;
    }
    
    if (!_rivals.user2.is_answered) {
        _rivals.user2.is_resigned = true;
    }
    
    if ((!_rivals.user1.is_resigned) && (!_rivals.user2.is_resigned)) {
        // dont send notification both of users are offline
        // finish the game with no winner
        mlog.log_debug("both of users timed out");
    }
    else if (_rivals.user1.is_resigned) {
        // send notification about timeout to active user
        mlog.log_debug("notification async");
        Requests::send_notification_async(_rivals.user2.socket, REQ_GAME_OPPONENT_TIMEOUT, "");
    }
    else if (_rivals.user2.is_resigned) {
        // send notification about timeout to active user
        mlog.log_debug("notification async");
        Requests::send_notification_async(_rivals.user1.socket, REQ_GAME_OPPONENT_TIMEOUT, "");
    }
}

void Game::set_timer() {
    _timer.set(GAME_TIMEOUT, this, &Game::timeout_func);
}

void Game::start_timer() {
    _rivals.user1.is_answered = false;
    _rivals.user2.is_answered = false;
    _timer.start();
}

void Game::stop_timer() {
    _timer.stop();
}

time_t Game::check_timer() {
    return _timer.check();
}

void Game::set_answer(uint64_t uid) {
    if (_rivals.user1.uid == uid) {
        _rivals.user1.is_answered = true;
    }
    else {
        _rivals.user2.is_answered = true;
    }
    
    if (_rivals.user1.is_answered && _rivals.user2.is_answered) {
        _timer.stop();
    }
}

void Game::resign(uint64_t uid) {
    if (_rivals.user1.uid == uid) {
        _rivals.user1.is_resigned = true;
        
        stop_timer();
        
        // send opponent resigned notification to the opponent
        Requests::send_notification_async(_rivals.user2.socket, REQ_GAME_OPPONENT_RESIGNED, "");
    }
    else {
        _rivals.user2.is_resigned = true;
        
        // send opponent resigned notification to the opponent
        Requests::send_notification_async(_rivals.user1.socket, REQ_GAME_OPPONENT_RESIGNED, "");
    }
}

bool Game::is_answered(uint64_t uid) {
    if (_rivals.user1.uid == uid)
        return _rivals.user1.is_answered;
    
    return _rivals.user2.is_answered;
}

ErrorCodes Game::check_answer(string data) {
    nlohmann::json answer_json;
    if (!nlohmann::json::accept(data)) {
        return ERR_GAME_WRONG_PACKET;
    }
    
    answer_json = nlohmann::json::parse(data);
    if (answer_json.find("answer") == answer_json.end()) {
        return ERR_GAME_WRONG_PACKET;
    }
    
    return ERR_SUCCESS;
}

bool Game::next_question() {
    _current_question++;
    if (_current_question == 5) {
        _current_tour++;
        _current_question = 0;
        return true;
    }
    
    return false;
}
