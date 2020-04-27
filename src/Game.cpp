#include <stdio.h>
#include <string>

#include "Game.h"
#include "Questions.h"
#include "Requests.h"
#include "debug.h"

using namespace std;

#define GAME_TIMEOUT 30

Game::Game(int game_id, Rivals rivals) {
    _start_dt = 0;
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
    }
    else if (_rivals.user2.uid == uid) {
        _rivals.user2.accept = true;
    }
    
    if (_rivals.user1.accept && _rivals.user2.accept) {
        _state = GAME_RIVALS_READY;
        return true;
    }
    
    return false;
}

ErrorCodes Game::is_ready(time_t start, bool is_blocking) {
    if (!is_blocking) {
        if (_state == GAME_RIVALS_READY)
            return ERR_SUCCESS;
        
        return ERR_GAME_START_FAIL;
    }
    
    while(_state != GAME_RIVALS_READY) {
        // wait
        if (time(NULL) > (start + GAME_START_TIMEOUT)) {
            return ERR_GAME_START_TIMEOUT;
        }
    }
    
    return ERR_SUCCESS;
}

time_t Game::get_start_dt() {
    return _start_dt;
}

void Game::start_game() {
    // get questions from db
    _questions = Questions::get_instance()->get_question(5);
    _start_dt = time(0);
    _state = GAME_QUESTIONS_READY;
    
    set_timer();
    start_timer();
}

string Game::get_questions() {
    return _questions;
}

void Game::timeout_func(void *arg) {
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
    _timer.set(GAME_TIMEOUT, this);
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