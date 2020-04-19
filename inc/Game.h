#ifndef _GAME_H_
#define _GAME_H_

#include <time.h>
#include "errors.h"

#define GAME_START_TIMEOUT 20

typedef enum {
    GAME_WAITING_FOR_ACCEPTIONS,
    GAME_RIVALS_READY,
    GAME_QUESTIONS_READY,
    GAME_STARTED,
    GAME_FINISHED
}GameState;

typedef struct {
    uint64_t uid;
    int socket;
    bool accept;
}GameUser;

typedef struct {
    GameUser user1;
    GameUser user2;
}Rivals;

class Game {
    public:
    Game(int game_id, Rivals rivals);
    int get_game_id();
    Rivals get_rivals();
    void accept_game(uint64_t uid);
    ErrorCodes is_ready(time_t start, bool is_blocking);
    void start_game(uint64_t uid);
    std::string get_questions();
    time_t get_start_dt();
    GameUser get_opponent(uint64_t uid);
    
    private:
    int _game_uid;
    time_t _start_dt;
    Rivals _rivals;
    GameState _state;
    std::string _questions;
};

#endif // _GAME_H_