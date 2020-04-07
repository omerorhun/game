#ifndef _MATCHER_H_
#define _MATCHER_H_

#include <vector>
#include <thread>
#include <mutex>

#include <ev.h>

#include "constants.h"

typedef struct {
    int uid;
    int op_uid;
}UserMatchInfo;
class Matcher {
    public:
    Matcher();
    void add(UserMatchInfo *user);
    void remove(UserMatchInfo *user);
    UserMatchInfo *lookup(int uid);
    void start_loop(struct ev_loop *loop);

    static Matcher *_p_instance;
    static Matcher *get_instance();
    std::thread *_p_matcher_thread;
    
    private:
    struct ev_loop *_p_loop;
    ev_async *_p_watcher;
    static std::vector<UserMatchInfo *> _waiting_matches;
    static void find_match_cb(struct ev_loop *loop, ev_async *watcher, int revents);
};

#endif // _MATCHER_H_