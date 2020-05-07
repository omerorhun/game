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
    uint8_t categories[3] = {
        _rivals.user1.category,
        _rivals.user2.category,
        0 // TODO: select random category
    };
    
    nlohmann::json tours;
    
    for (int i = 0; i < 3; i++) {
        // get this tour's questions from database by category
        string tour = Questions::get_instance()->get_question(5, categories[i]);
        
        // convert to tour info as json object
        nlohmann::json tour_json = nlohmann::json::parse(tour);
        
        // add category info to every tour info
        tour_json["category"] = categories[i];
        
        // add tour info under "tours" key
        tours["tours"].push_back(tour_json);
    }
    
    _questions = tours.dump();
    
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
        Requests::send_notification_async(_rivals.user2.socket, Requests::REQ_GAME_OPPONENT_TIMEOUT, "");
    }
    else if (_rivals.user2.is_resigned) {
        // send notification about timeout to active user
        mlog.log_debug("notification async");
        Requests::send_notification_async(_rivals.user1.socket, Requests::REQ_GAME_OPPONENT_TIMEOUT, "");
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
    
    stop_timer();
    
    if (_rivals.user1.uid == uid) {
        _rivals.user1.is_resigned = true;
        
        // send opponent resigned notification to the opponent
        Requests::send_notification_async(_rivals.user2.socket, Requests::REQ_GAME_OPPONENT_RESIGNED, "");
    }
    else {
        _rivals.user2.is_resigned = true;
        
        // send opponent resigned notification to the opponent
        Requests::send_notification_async(_rivals.user1.socket, Requests::REQ_GAME_OPPONENT_RESIGNED, "");
    }
}

bool Game::is_answered(uint64_t uid) {
    if (_rivals.user1.uid == uid)
        return _rivals.user1.is_answered;
    
    return _rivals.user2.is_answered;
}
