#ifndef _MATCHER_H_
#define _MATCHER_H_

#include <vector>
#include <thread>
#include <mutex>

#include <ev.h>
#include "Timer.h"
#include "constants.h"

typedef struct {
    uint64_t uid;
    int socket;
    Timer timer;
}UserMatchInfo;

typedef struct {
    UserMatchInfo user1;
    UserMatchInfo user2;
}MatchResult;

class Matcher {
    public:
    Matcher();
    void add(UserMatchInfo *user);
    void remove(UserMatchInfo *user);
    UserMatchInfo *lookup(uint64_t uid);
    void start_loop(struct ev_loop *loop);

    static Matcher *_p_instance;
    static Matcher *get_instance();
    std::thread *_p_matcher_thread;
    
    static void timeout_func(uint64_t uid);
    
    private:
    static struct ev_loop *_p_loop;
    static ev_async _find_match_watcher;
    static ev_async _create_game_watcher;
    static std::vector<UserMatchInfo *> _waiting_matches;
    static std::vector<MatchResult> _match_results;
    static void find_match_cb(struct ev_loop *loop, ev_async *watcher, int revents);
    static void match_cb(struct ev_loop *loop, ev_async *watcher, int revents);
};

#endif // _MATCHER_H_