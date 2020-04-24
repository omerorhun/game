#ifndef _TIMER_H_
#define _TIMER_H_

#include <thread>

typedef void (*funcPtr)(void *arg);

class Timer {
    public:
    Timer();
    Timer(const Timer &obj);
    Timer operator=(const Timer &obj);
    ~Timer();
    
    void start();
    void stop();
    //void set(time_t sec, funcPtr, void *arg);
    void set(time_t sec, void *game);
    void set(time_t sec, uint64_t uid);
    time_t check();
    
    private:
    time_t _start_dt;
    time_t _timeout;
    bool _is_active;
    void *_game;
    uint64_t _uid_match;
    funcPtr _p_func;
    void *_arg;
    void loop();
    std::thread *_thread;
};

#endif // _TIMER_H_