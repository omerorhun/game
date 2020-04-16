#include <stdio.h>
#include <stdlib.h>

#include "GameService.h"
#include "debug.h"

using namespace std;

vector<Game> GameService::_games;
int GameService::_s_game_count = 0;
GameService *GameService::_p_instance = NULL;

GameService::GameService() {
    if (_p_instance == NULL) {
        _p_instance = this;
    }
}

GameService *GameService::get_instance() {
    if (_p_instance == NULL) {
        GameService();
    }
    
    return _p_instance;
}

int GameService::create_game(Rivals rivals) {
    mlog.log_info("create game");
    
    mlog.log_debug("%d - %d", rivals.user1.uid, rivals.user2.uid);
    int game_id = 0;
    
    // if not create new game
    Game game(_s_game_count + 1, rivals);
    _games.push_back(game);
    game_id = _games[_s_game_count++].get_game_id();
    
    return game_id;
}

ErrorCodes GameService::accept_game(int game_id, int user_id) {
    Game *game = lookup(user_id);
    game->accept_game(user_id);
}

Game *GameService::lookup(int uid) {
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

int GameService::get_game_id(int user_id) {
    Game *game = lookup(user_id);
    if (game == NULL)
        return 0;
    
    mlog.log_debug("game found");
    return game->get_game_id();
}
