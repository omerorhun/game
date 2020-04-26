#include "Matcher.h"
#include "Requests.h"
#include "RegistryService.h"

#include <ev.h>

#include <algorithm>
#include <pthread.h>
#include <thread>
#include <unistd.h>

#include "debug.h"
#include "json.hpp"

using namespace std;

Matcher *Matcher::_p_instance = NULL;
vector<UserMatchInfo *> Matcher::_waiting_matches;
vector<MatchResult> Matcher::_match_results;

struct ev_loop *Matcher::_p_loop;
ev_async Matcher::_create_game_watcher;
ev_async Matcher::_find_match_watcher;

mutex g_waiting_list_mtx;
mutex g_match_result_mtx;

Matcher::Matcher() {
    if (_p_instance == NULL) {
        _p_instance = this;
        
        _p_loop = ev_loop_new(0);
        
        // start main watcher
        ev_async_init(&_find_match_watcher, find_match_cb);
        ev_async_start(_p_loop, &_find_match_watcher);
        
        // start callback watcher
        ev_async_init(&_create_game_watcher, match_cb);
        ev_async_start(_p_loop, &_create_game_watcher);
        
        // start main event listener
        _p_matcher_thread = new thread(&Matcher::start_loop, this, _p_loop);
    }
}

void Matcher::find_match_cb(struct ev_loop *loop, ev_async *watcher, int revents) {
    int remaining = _waiting_matches.size();
    
    mlog.log_debug("remaining: %d", remaining);
    while (_waiting_matches.size() > 1) {
        g_waiting_list_mtx.lock();
        
        // some users could be canceled their match requests
        if ((remaining = _waiting_matches.size()) <= 1)
            break;
        
        // find random opponent here
        int op_idx = rand()%(remaining - 1) + 1;
        
        _waiting_matches[0]->timer.stop();
        _waiting_matches[op_idx]->timer.stop();
        
        MatchResult result;
        result.user1 = *_waiting_matches[0];
        result.user2 = *_waiting_matches[op_idx];
        
        delete _waiting_matches[0];
        delete _waiting_matches[op_idx];
        
        g_match_result_mtx.lock();
        _match_results.push_back(result);
        g_match_result_mtx.unlock();
        
        // first, remove opponent's uid from the list
        _waiting_matches.erase(_waiting_matches.begin() + op_idx);
        
        // then, remove this uid from the list
        _waiting_matches.erase(_waiting_matches.begin());
        
        if (ev_async_pending(&_create_game_watcher) == false)
            ev_async_send(_p_loop, &_create_game_watcher);
        
        g_waiting_list_mtx.unlock();
    }
}

void Matcher::match_cb(struct ev_loop *loop, ev_async *watcher, int revents) {
    MatchResult result;
    
    g_match_result_mtx.lock();
    
    result = _match_results.back();
    _match_results.pop_back();
    g_match_result_mtx.unlock();
    
    // create new game
    int game_id = 0;
    Rivals riv;
    riv.user1.uid = result.user1.uid;
    riv.user1.socket = result.user1.socket;
    riv.user1.accept = false;
    riv.user2.uid = result.user2.uid;
    riv.user2.socket = result.user2.socket;
    riv.user2.accept = false;
    game_id = GameService::get_instance()->create_game(riv);
    // is game id must be saved?
    
    // match success
    mlog.log_debug("%lu matched with %lu", result.user1.uid, result.user2.uid);
    
    // 3- get opponent info :
    RegistryInfo user1_info = RegistryService::get_instance()->get_user_data(riv.user1.uid);
    RegistryInfo user2_info = RegistryService::get_instance()->get_user_data(riv.user2.uid);
    nlohmann::json user1_json, user2_json;
    
    user1_json["id"] = user1_info.uid;
    user1_json["fb_id"] = user1_info.fb_id;
    user1_json["name"] = user1_info.name;
    user1_json["url"] = user1_info.picture_url;
    user1_json["game_id"] = game_id;
    
    user2_json["id"] = user2_info.uid;
    user2_json["fb_id"] = user2_info.fb_id;
    user2_json["name"] = user2_info.name;
    user2_json["url"] = user2_info.picture_url;
    user2_json["game_id"] = game_id;
    
    Requests::send_notification_async(riv.user1.socket, REQ_MATCH, user2_json.dump());
    Requests::send_notification_async(riv.user2.socket, REQ_MATCH, user1_json.dump());
}

void Matcher::start_loop(struct ev_loop *loop) {
    // start the event listener
    ev_run(loop, 0);
}

Matcher *Matcher::get_instance() {
    return _p_instance;
}

ErrorCodes Matcher::add(UserMatchInfo *user) {
    UserMatchInfo *res = lookup(user->uid);
    if (res != NULL) {
        return ERR_MATCH_WAITING;
    }
    
    // add user to the waiting list
    g_waiting_list_mtx.lock();
    mlog.log_debug("adding to queue");
    _waiting_matches.push_back(user);
    
    g_waiting_list_mtx.unlock();
    
    // send event to find match listener
    if (ev_async_pending(&_find_match_watcher) == false)
        ev_async_send(_p_loop, &_find_match_watcher);
    
    return ERR_SUCCESS;
}

void Matcher::remove(UserMatchInfo *user) {
    g_waiting_list_mtx.lock();
    //auto it = find(_waiting_matches.begin(), _waiting_matches.end(), user);
    for (int i = 0; i < _waiting_matches.size(); i++) {
        if (_waiting_matches[i]->uid == user->uid) {
            _waiting_matches.erase(_waiting_matches.begin() + i);
            break;
        }
    }
    
    g_waiting_list_mtx.unlock();
}

UserMatchInfo *Matcher::lookup(uint64_t uid) {
    UserMatchInfo *ret = NULL;
    
    g_waiting_list_mtx.lock();
    for (int i = 0; i < _waiting_matches.size(); i++) {
        if (_waiting_matches[i]->uid == uid) {
            ret = _waiting_matches[i];
            break;
        }
    }
    g_waiting_list_mtx.unlock();
    
    return ret;
}

void Matcher::timeout_func(uint64_t uid) {
    UserMatchInfo *user = Matcher::get_instance()->lookup(uid);
    if (user == NULL)
        return;
    
    int socket = user->socket;
    Matcher::get_instance()->remove(user);
    
    char data = ERR_REQ_MATCH_FAIL;
    Requests::send_notification_async(socket, REQ_ERROR, string(&data));
}
