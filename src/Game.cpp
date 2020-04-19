#include <stdio.h>
#include <string>

#include "Game.h"
#include "Questions.h"

using namespace std;

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
    else {
        return _rivals.user1;
    }
}

void Game::accept_game(uint64_t uid) {
    if (_rivals.user1.uid == uid) 
        _rivals.user1.accept = true;
    else if (_rivals.user2.uid == uid)
        _rivals.user2.accept = true;
    
    if (_rivals.user1.accept && _rivals.user2.accept)
        _state = GAME_RIVALS_READY;
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

void Game::start_game(uint64_t uid) {
    if (uid == _rivals.user1.uid) {
        
        // get questions from db
        _questions = Questions::get_instance()->get_question(5);
        _start_dt = time(0);
        _state = GAME_QUESTIONS_READY;
    }
}

string Game::get_questions() {
    // TODO: add timeout
    while (_state != GAME_QUESTIONS_READY);
    
    return _questions;
}