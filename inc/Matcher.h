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
}User;

class Matcher {
    public:
    Matcher();
    void add(User *user);
    void start_loop(struct ev_loop *loop);

    static Matcher *_p_instance;
    static Matcher *get_instance();
    std::thread *main_th;
    std::mutex lock_main;
    
    private:
    struct ev_loop *_main_loop;
    ev_async *_main_watcher;
    std::vector<User *> _waiting_matches;
    static void find_match_cb(struct ev_loop *loop, ev_async *watcher, int revents);
};

#endif // _MATCHER_H_