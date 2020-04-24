#include "Timer.h"
#include "Game.h"
#include "Matcher.h"
#include "debug.h"

using namespace std;

Timer::Timer() {
    _start_dt = 0;
    _timeout = 0;
    _is_active = false;
    _thread = NULL;
    _p_func = NULL;
    _uid_match = 0;
    _game = NULL;
    mlog.log_debug("normal constructor _thread: %p", _thread);
}

Timer::Timer(const Timer &obj) {
    _start_dt = 0;
    _timeout = 0;
    _is_active = false;
    _thread = NULL;
    _p_func = NULL;
    _uid_match = 0;
    _game = NULL;
    mlog.log_debug("copy constructor _thread: %p", _thread);
}

Timer Timer::operator=(const Timer &obj) {
    Timer timer;
    timer._start_dt = 0;
    timer._timeout = 0;
    timer._is_active = false;
    timer._thread = NULL;
    timer._p_func = NULL;
    timer._uid_match = 0;
    timer._game = NULL;
    
    return timer;
}

Timer::~Timer() {
    mlog.log_debug("_thread: %p", _thread);
    if (_thread != NULL) {
        stop();
        mlog.log_debug("timer destructor\n");
        
        delete _thread;
        _thread = NULL;
    }
}

void Timer::set(time_t sec, void *game) {
    _thread = new thread();
    mlog.log_debug("timer set _thread: %p", _thread);
    _timeout = sec;
    _game = game;
}

void Timer::set(time_t sec, uint64_t uid) {
    _thread = new thread();
    mlog.log_debug("timer set _thread: %p", _thread);
    _timeout = sec;
    _uid_match = uid;
}

void Timer::start() {
    if ((_game == NULL) && (_uid_match == 0))
        return;
    
    _start_dt = time(NULL);
    _is_active = true;
    *_thread = thread(&Timer::loop, this);
}

void Timer::stop() {
    if (!_is_active)
        return;
    
    _is_active = false;
    mlog.log_debug("stop the timer %p", _thread);
    _thread->detach();
}

void Timer::loop() {
    
    while (time(NULL) < (_start_dt + _timeout)) {
        if (!_is_active) {
            return;
        }
    }
    
    mlog.log_debug("timeout");
    if (_game != NULL)
        ((Game *)_game)->timeout_func(NULL);
    
    if (_uid_match != 0) {
        Matcher::timeout_func(_uid_match);
    }
}

time_t Timer::check() {
    
    if (!_is_active)
        return 0;
    
    return _start_dt + _timeout - time(NULL);
}
