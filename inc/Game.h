#ifndef _GAME_H_
#define _GAME_H_

#include <time.h>
#include "errors.h"
#include "Timer.h"

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
    bool accept = false;
    uint8_t category;
    bool is_resigned = false;
    bool is_answered = false;
}GameUser;

typedef struct {
    GameUser user1;
    GameUser user2;
}Rivals;

class Requests;

class Game {
    public:
    Game();
    Game(int game_id, Rivals rivals);
    
    void start_game();
    bool accept_game(uint64_t uid);
    void resign(uint64_t uid);
    
    int get_game_id();
    Rivals get_rivals();
    
    void set_category(uint64_t uid, uint8_t category);
    std::string get_questions();
    GameUser get_opponent(uint64_t uid);
    
    void set_answer(uint64_t uid);
    bool is_answered(uint64_t uid);
    
    // timer
    void timeout_func();
    void set_timer();
    void start_timer();
    void stop_timer();
    time_t check_timer();
    
    private:
    int _game_uid;
    uint64_t game_count;
    Rivals _rivals;
    uint8_t _current_question;
    
    Timer<Game> _timer;
    GameState _state;
    std::string _questions;
};

#endif // _GAME_H_