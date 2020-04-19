/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <mutex>

#define LOG_VERSION "0.1.0"
#define LOG_USE_COLOR

typedef void (*log_LockFn)(void *udata, int lock);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#define log_hex(header, buffer, len) log_dhex(LOG_DEBUG, header, buffer, len, __FILE__, __LINE__)

class Dlogger {
    public:
    Dlogger();
    Dlogger(uint64_t uid);
    Dlogger(std::string name);
    ~Dlogger();
    void log_set_udata(void *udata);
    void log_set_lock(log_LockFn fn);
    void log_set_fp(FILE *fp);
    void log_reset_fp();
    void log_set_level(int level);
    void log_set_quiet(int enable);
    
    void log_log(int level, const char *file, int line, const char *fmt, ...);
    void log_dhex(int level, const char *header, char *buffer, uint16_t len, const char *file, int line);
    
    private:
    void *_udata;
    std::mutex _lock;
    FILE *_fp;
    int _level;
    int _quiet;
    
    static const char *level_names[];

#ifdef LOG_USE_COLOR
    static const char *level_colors[];
#endif
    
    FILE *generate_log_file();
    void lock();
    void unlock();
    
};

extern Dlogger mlog;

#endif