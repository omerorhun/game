#ifndef _GAME_SERVICE_H_
#define _GAME_SERVICE_H_

#include "Game.h"
#include "errors.h"

#include <list>
#include <ev.h>
#include <thread>
#include <string>
#include <mutex>

class GameService {
    
    public:
        GameService();
        static GameService *get_instance();
        
        Game *create_game(Rivals rivals);
        ErrorCodes accept_game(int game_id, uint64_t uid);
        ErrorCodes start_game(int game_id);
        ErrorCodes finish_game_with_uid(uint64_t uid);
        ErrorCodes finish_game(Game *game, std::string results);
        ErrorCodes remove_game(int game_id);
        Game *lookup_by_uid(uint64_t uid);
        Game *lookup(int game_id);
        int get_game_id(uint64_t uid);
        
    private:
        static GameService *_ps_instance;
        int _game_id_offset;
        std::list<Game> _games;
        std::mutex _mtx_add;
        
        void watchdog(); // for removing finished games
};

#endif // _GAME_SERVICE_H_