#ifndef _GAME_H_
#define _GAME_H_

typedef enum {
    GAME_WAITING_FOR_ACCEPTIONS,
    GAME_RIVALS_READY,
    GAME_QUESTIONS_READY,
    GAME_STARTED,
    GAME_FINISHED
}GameState;

typedef struct {
    int uid;
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
    void accept_game(int uid);
    bool is_ready();
    void start_game(int uid);
    std::string get_questions();
    
    private:
    int _game_uid;
    Rivals _rivals;
    GameState _state;
    std::string _questions;
    
};

#endif // _GAME_H_