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
        static GameService *_p_instance;
        static GameService *get_instance();
        
        int create_game(Rivals rivals);
        ErrorCodes accept_game(int game_id, int user_id);
        ErrorCodes start_game(int game_id);
        Game *lookup(int uid);
        int get_game_id(int uid);
        
    private:
        static int _s_game_count;
        static std::vector<Game> _games;
};

#endif // _GAME_SERVICE_H_