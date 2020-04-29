#ifndef _MATCHER_H_
#define _MATCHER_H_

#include <vector>
#include <thread>
#include <mutex>

#include <ev.h>
#include "Timer.h"
#include "constants.h"
#include "errors.h"

typedef struct {
    uint64_t uid;
    int socket;
    time_t start_time;
}UserMatchInfo;

typedef struct {
    UserMatchInfo user1;
    UserMatchInfo user2;
}MatchResult;

class Matcher {
    public:
    Matcher();
    ~Matcher();
    ErrorCodes add(UserMatchInfo *user);
    void remove(UserMatchInfo *user);
    UserMatchInfo *lookup(uint64_t uid);
    
    
    static Matcher *get_instance();
    
    void timeout_func(uint64_t uid);
    
    private:
    static Matcher *_p_instance;
    std::thread *_p_matcher_thread;
    std::thread *_p_watchdog_thread;
    
    struct ev_loop *_p_loop;
    ev_async _find_match_watcher;
    ev_async _create_game_watcher;
    std::vector<UserMatchInfo *> _waiting_matches;
    std::vector<MatchResult> _match_results;
    static void find_match_cb(struct ev_loop *loop, ev_async *watcher, int revents);
    static void match_cb(struct ev_loop *loop, ev_async *watcher, int revents);
    void start_loop(struct ev_loop *loop);
    void watchdog();
};

#endif // _MATCHER_H_