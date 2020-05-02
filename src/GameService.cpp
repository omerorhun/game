#include <stdio.h>
#include <stdlib.h>

#include "GameService.h"
#include "debug.h"

using namespace std;

vector<Game> GameService::_games;
int GameService::_s_game_count = 0;
GameService *GameService::_ps_instance = NULL;

GameService::GameService() {
    if (_ps_instance == NULL) {
        _ps_instance = this;
    }
}

GameService *GameService::get_instance() {
    if (_ps_instance == NULL) {
        _ps_instance = new GameService();
    }
    
    return _ps_instance;
}

Game *GameService::create_game(Rivals rivals) {
    mlog.log_info("create game");
    
    mlog.log_debug("%lu - %lu", rivals.user1.uid, rivals.user2.uid);
    
    // if not create new game
    // TODO: determine appropriate game id
    Game game(_s_game_count + 1, rivals);
    _games.push_back(game);
    
    return &_games[_games.size()];
}

ErrorCodes GameService::accept_game(int game_id, uint64_t uid) {
    Game *game = lookup(uid);
    game->accept_game(uid);
}

ErrorCodes GameService::remove_game(int game_id) {
    for (int i = 0; i < _games.size(); i++) {
        if (_games[i].get_game_id() == game_id) {
            _games.erase(_games.begin() + i);
            break;
        }
    }
    
    return ERR_SUCCESS;
}

ErrorCodes GameService::finish_game(int game_id) {
    return remove_game(game_id);
}

ErrorCodes GameService::finish_game_with_uid(uint64_t uid) {
    int game_id = get_game_id(uid);
    if (game_id == 0) {
        // game no exists, so may be already removed
        return ERR_SUCCESS;
    }
    
     return remove_game(game_id);
}

Game *GameService::lookup_by_uid(uint64_t uid) {
    Game *game = NULL;
    
    for (int i = 0; i < _games.size(); i++) {
        Rivals riv = _games[i].get_rivals();
        if ((riv.user1.uid == uid) || 
            (riv.user2.uid == uid)) {
            game = &_games[i];
            break;
        }
    }
    
    return game;
}

Game *GameService::lookup(int game_id) {
    Game *game = NULL;
    
    for (int i = 0; i < _games.size(); i++) {
        if (_games[i].get_game_id() == game_id) {
            game = &_games[i];
            break;
        }
    }
    
    return game;
}

int GameService::get_game_id(uint64_t uid) {
    Game *game = lookup(uid);
    if (game == NULL)
        return 0;
    
    mlog.log_debug("game found");
    return game->get_game_id();
}
