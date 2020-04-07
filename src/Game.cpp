#include <stdio.h>
#include <string>

#include "Game.h"

using namespace std;

Game::Game(int game_id, Rivals rivals) {
    _game_uid = game_id;
    _rivals.user1 = rivals.user1;
    _rivals.user2 = rivals.user2;
}

int Game::get_game_id() {
    return _game_uid;
}

Rivals Game::get_rivals() {
    return _rivals;
}

void Game::accept_game(int uid) {
    if (_rivals.user1.uid == uid) 
        _rivals.user1.accept = true;
    else if (_rivals.user2.uid == uid)
        _rivals.user2.accept = true;
    
    if (_rivals.user1.accept && _rivals.user2.accept)
        _state = GAME_RIVALS_READY;
}

bool Game::is_ready() {
    if (_state == GAME_RIVALS_READY)
        return true;
    
    return false;
}

void Game::start_game(int uid) {
    if (uid == _rivals.user1.uid) {
        
        // get questions from db
        _state = GAME_QUESTIONS_READY;
    }
}

string Game::get_questions() {
    if (_state == GAME_QUESTIONS_READY) {
        return _questions;
    }
    
    return string("");
}