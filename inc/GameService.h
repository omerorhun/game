#ifndef _GAME_SERVICE_H_
#define _GAME_SERVICE_H_

#include <vector>
#include <ev.h>
#include <thread>

#include "Game.h"
#include "errors.h"

class GameService {
    
    public:
        GameService();
        static GameService *get_instance();
        
        Game *create_game(Rivals rivals);
        ErrorCodes accept_game(int game_id, uint64_t uid);
        ErrorCodes start_game(int game_id);
        ErrorCodes finish_game_with_uid(uint64_t uid);
        ErrorCodes finish_game(int game_id);
        ErrorCodes remove_game(int game_id);
        Game *lookup_by_uid(uint64_t uid);
        Game *lookup(int game_id);
        int get_game_id(uint64_t uid);
        
    private:
        static GameService *_ps_instance;
        static int _s_game_count;
        static std::vector<Game> _games;
};

#endif // _GAME_SERVICE_H_