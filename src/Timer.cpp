#include "Timer.h"
#include "Game.h"
#include "Matcher.h"
#include "debug.h"

using namespace std;

template <typename __OBJ>
Timer<__OBJ>::Timer() {
    _start_dt = 0;
    _timeout = 0;
    _is_active = false;
    _thread = NULL;
    _p_callback = NULL;
    _obj = NULL;
}

template <typename __OBJ>
Timer<__OBJ>::Timer(const Timer &obj) {
    _start_dt = 0;
    _timeout = 0;
    _is_active = false;
    _thread = NULL;
    _p_callback = NULL;
    _obj = NULL;
}

template <typename __OBJ>
Timer<__OBJ> Timer<__OBJ>::operator=(const Timer<__OBJ> &obj) {
    Timer timer;
    timer._start_dt = 0;
    timer._timeout = 0;
    timer._is_active = false;
    timer._thread = NULL;
    timer._p_callback = NULL;
    timer._obj = NULL;
    
    return timer;
}

template <typename __OBJ>
Timer<__OBJ>::~Timer() {
    if (_thread != NULL) {
        stop();
        delete _thread;
        _thread = NULL;
    }
}

template <typename __OBJ>
void Timer<__OBJ>::set(time_t sec, __OBJ *obj, void (__OBJ::*pfunc)()) {
    _thread = new thread();
    _timeout = sec;
    _obj = obj;
    _p_callback = pfunc;
}

template <typename __OBJ>
void Timer<__OBJ>::start() {
    if (_obj == NULL)
        return;
    
    _start_dt = time(NULL);
    _is_active = true;
    *_thread = thread(&Timer::loop, this);
}

template <typename __OBJ>
void Timer<__OBJ>::stop() {
    if (!_is_active)
        return;
    
    _is_active = false;
    _thread->detach();
}

template <typename __OBJ>
void Timer<__OBJ>::loop() {
    while (time(NULL) < (_start_dt + _timeout)) {
        if (!_is_active) {
            return;
        }
    }
    
    (_obj->*_p_callback)();
}

template <typename __OBJ>
time_t Timer<__OBJ>::check() {
    if (!_is_active)
        return 0;
    
    return _start_dt + _timeout - time(NULL);
}

template class Timer<Game>;
