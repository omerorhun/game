#include <stdio.h>
#include <stdlib.h>

#include "GameService.h"
#include "GameDAL.h"
#include "debug.h"
#include "json.hpp"

using namespace std;

GameService *GameService::_ps_instance = NULL;

GameService::GameService() {
    if (_ps_instance == NULL) {
        _ps_instance = this;
        _game_id_offset = 0;
    }
}

GameService *GameService::get_instance() {
    if (_ps_instance == NULL) {
        _ps_instance = new GameService();
    }
    
    return _ps_instance;
}

Game *GameService::create_game(Rivals rivals) {
    mlog.log_info("create game %lu - %lu", rivals.user1.uid, rivals.user2.uid);
    
    // TODO: determine appropriate game id
    // TODO: add mutex while incrementing offset
    _mtx_add.lock();
    _game_id_offset++;
    Game game(_game_id_offset, rivals);
    _games.push_back(game);
    Game *p_game = &_games.back();
    _mtx_add.unlock();
    
    return p_game;
}

ErrorCodes GameService::accept_game(int game_id, uint64_t uid) {
    Game *game = lookup(uid);
    game->accept_game(uid);
}

ErrorCodes GameService::remove_game(int game_id) {
    for (auto it = _games.begin(); it != _games.end(); it++) {
        if (it->get_game_id() == game_id) {
            _games.erase(it);
            break;
        }
    }
    
    return ERR_SUCCESS;
}

ErrorCodes GameService::finish_game(Game *game, string results) {
    mlog.log_debug("results debug");
    GameDAL *dal = GameDAL::get_instance();
    Rivals riv = game->get_rivals();
    
    nlohmann::json results_json = nlohmann::json::parse(results);
    uint64_t winner = results_json["winner"];
    nlohmann::json user1 = results_json["users"].at(0);
    nlohmann::json user2 = results_json["users"].at(1);
    
    UserStatisticsInfo usi;
    uint8_t category;
    for (int i = 0; i < 3; i++) {
        
        usi.uid = user1["uid"];
        usi.right = user1["tours"].at(i)["right"];
        usi.wrong = user1["tours"].at(i)["wrong"];
        category = user1["tours"].at(i)["category"];
        GameDAL::get_instance()->update_user_stat(usi.uid, category, usi);
        
        usi.uid = user2["uid"];
        usi.right = user2["tours"].at(i)["right"];
        usi.wrong = user2["tours"].at(i)["wrong"];
        category = user2["tours"].at(i)["category"];
        GameDAL::get_instance()->update_user_stat(usi.uid, category, usi);
    }
    
    mlog.log_debug("users' statitsics updated");
    
    return remove_game(game->get_game_id());
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
    
    for (auto it = _games.begin(); it != _games.end(); it++) {
        Rivals riv = it->get_rivals();
        if ((riv.user1.uid == uid) || 
            (riv.user2.uid == uid))
        {
            game = &*it;
            break;
        }
    }
    
    return game;
}

Game *GameService::lookup(int game_id) {
    Game *game = NULL;
    for (auto it = _games.begin(); it != _games.end(); it++) {
        if (it->get_game_id() == game_id) {
            game = &*it;
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
