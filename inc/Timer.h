#ifndef _TIMER_H_
#define _TIMER_H_

#include <thread>

template <typename __OBJ>
class Timer {
    public:
    Timer();
    Timer(const Timer &obj);
    Timer operator=(const Timer &obj);
    ~Timer();
    
    void start();
    void stop();
    
    //void set(time_t sec, __OBJ *obj);
    void set(time_t sec, __OBJ *obj, void (__OBJ::*pfunc)());
    
    time_t check();
    
    private:
    time_t _start_dt;
    time_t _timeout;
    bool _is_active;
    __OBJ *_obj;
    void (__OBJ::*_p_callback)();
    
    void loop();
    std::thread *_thread;
};

#endif // _TIMER_H_