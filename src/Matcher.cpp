#include "Matcher.h"
#include <ev.h>

#include <algorithm>
#include <pthread.h>
#include <thread>
#include <unistd.h>

using namespace std;

Matcher *Matcher::_p_instance = NULL;
vector<UserMatchInfo *> Matcher::_waiting_matches;
mutex g_mtx;

Matcher::Matcher() {
    if (_p_instance == NULL) {
        _p_instance = this;
        
        _p_watcher = new ev_async();
        _p_loop = ev_loop_new(0);
        
        // start main watcher
        ev_async_init(_p_watcher, find_match_cb);
        ev_async_start(_p_loop, _p_watcher);
        
        // start main event listener
        _p_matcher_thread = new thread(&Matcher::start_loop, this, _p_loop);
    }
}

void Matcher::find_match_cb(struct ev_loop *loop, ev_async *watcher, int revents) {
    int remaining = _waiting_matches.size();
    
    printf("remaining: %d\n", remaining);
    while (_waiting_matches.size() > 1) {
        g_mtx.lock();
        
        // some users could be canceled their match requests
        if ((remaining = _waiting_matches.size()) <= 1)
            break;
        
        // find random opponent here
        int op_idx = rand()%(remaining - 1) + 1;
        
        _waiting_matches[0]->op_uid = _waiting_matches[op_idx]->uid;
        _waiting_matches[op_idx]->op_uid = _waiting_matches[0]->uid;
        
        // first, remove opponent's uid from the list
        _waiting_matches.erase(_waiting_matches.begin() + op_idx);
        
        // then, remove this uid from the list
        _waiting_matches.erase(_waiting_matches.begin());
        
        g_mtx.unlock();
    }
}

void Matcher::start_loop(struct ev_loop *loop) {
    // start the event listener
    ev_run(loop, 0);
}

Matcher *Matcher::get_instance() {
    return _p_instance;
}

void Matcher::add(UserMatchInfo *user) {
    
    // add user to the waiting list
    g_mtx.lock();
    _waiting_matches.push_back(user);
    g_mtx.unlock();
    
    // send event to find match listener
    if (ev_async_pending(_p_watcher) == false)
        ev_async_send(_p_loop, _p_watcher);
}

void Matcher::remove(UserMatchInfo *user) {
    //User usr = *user;
    g_mtx.lock();
    auto it = find(_waiting_matches.begin(), _waiting_matches.end(), user);
    _waiting_matches.erase(it);
    g_mtx.unlock();
}

UserMatchInfo *Matcher::lookup(int uid) {
    UserMatchInfo *ret = NULL;
    g_mtx.lock();
    for (int i = 0; i < _waiting_matches.size(); i++) {
        if (_waiting_matches[i]->uid == uid) {
            ret = _waiting_matches[i];
            break;
        }
    }
    g_mtx.unlock();
}
