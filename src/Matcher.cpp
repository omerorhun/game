#include "Matcher.h"
#include <ev.h>

#include <algorithm>
#include <pthread.h>
#include <thread>
#include <unistd.h>

using namespace std;

Matcher *Matcher::_p_instance = NULL;
mutex g_mtx;

Matcher::Matcher() {
    if (_p_instance == NULL) {
        _p_instance = this;
        
        _main_watcher = new ev_async();
        _main_loop = ev_loop_new(0);
        
        lock_main.unlock();
        
        // start main watcher
        ev_async_init(_main_watcher, find_match_cb);
        ev_async_start(_main_loop, _main_watcher);
        
        // start main event listener
        main_th = new thread(&Matcher::start_loop, this, _main_loop);
    }
}

void Matcher::find_match_cb(struct ev_loop *loop, ev_async *watcher, int revents) {
    Matcher *matcher = Matcher::get_instance();
    printf("%s\n", __func__);
    int remaining = matcher->_waiting_matches.size();
    printf("remaining: %d\n", remaining);
    while (remaining > 1) {
        // find random opponent here
        int opponent = rand()%(remaining - 1) + 1;
        
        User *first = matcher->_waiting_matches[0];
        User *second = matcher->_waiting_matches[opponent];
        
        first->op_uid = second->uid;
        second->op_uid = first->uid;
        
        g_mtx.lock();
        // first, remove opponent's uid from the list
        matcher->_waiting_matches.erase(matcher->_waiting_matches.begin() + opponent);
        
        // then, remove this uid from the list
        matcher->_waiting_matches.erase(matcher->_waiting_matches.begin());
        g_mtx.unlock();
        
        remaining -= 2;
    }
}

void Matcher::start_loop(struct ev_loop *loop) {
    // start the event listener
    ev_run(loop, 0);
}

Matcher *Matcher::get_instance() {
    return _p_instance;
}

void Matcher::add(User *user) {
    
    g_mtx.lock();
    // pass user info
    // add user to the waiting list
    _waiting_matches.push_back(user);
    g_mtx.unlock();
    
    // send find_match_watcher
    if (ev_async_pending(_main_watcher) == false)
        ev_async_send(_main_loop, _main_watcher);
}
