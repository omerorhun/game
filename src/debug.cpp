/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"

const char *Dlogger::level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };
const char *Dlogger::level_colors[] = {
    "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
    };

Dlogger::Dlogger(uint64_t uid) {
  _level = LOG_DEBUG;
  _fp = NULL;
  _quiet = 0;
  
  char file_name[32];
  memset(file_name, 0, 32);
  sprintf(file_name, "%lu_log.txt", uid);
  
  _fp = fopen(file_name, "a+");
}

Dlogger::Dlogger() {
  _level = LOG_DEBUG;
  _fp = NULL;
  _quiet = 0;
  
  _fp = generate_log_file();
}

Dlogger::Dlogger(std::string name) {
  _level = LOG_DEBUG;
  _fp = NULL;
  _quiet = 0;
    
  name.append("_log.txt");
  
  _fp = fopen(name.c_str(), "a+");
}

Dlogger::~Dlogger() {
  fclose(_fp);
}

void Dlogger::lock()   {
  _lock.lock();
}

void Dlogger::unlock() {
  _lock.unlock();
}

void Dlogger::log_set_udata(void *udata) {
  _udata = udata;
}

void Dlogger::log_set_lock(log_LockFn fn) {
  //_lock = fn;
}

void Dlogger::log_set_fp(FILE *fp) {
  _fp = fp;
}

void Dlogger::log_reset_fp() {
  if (_fp != NULL) {
    fclose(_fp);
    _fp = NULL;
  }
}

void Dlogger::log_set_level(int level) {
  _level = level;
}

void Dlogger::log_set_quiet(int enable) {
  _quiet = enable ? 1 : 0;
}

void Dlogger::log_log(int level, const char *file, int line, const char *fmt, ...) {
  if (level < _level) {
    return;
  }

  /* Acquire lock */
  lock();

  /* Get current time */
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  /* Log to stderr */
  if (!_quiet) {
    va_list args;
    char buf[16];
    buf[strftime(buf, sizeof(buf), "[%H:%M:%S]", lt)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(
      stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%4d:\x1b[0m ",
      buf, level_colors[level], level_names[level], file, line);
#else
    fprintf(stderr, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
#endif
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
  }

  /* Log to file */
  if (_fp) {
    va_list args;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    fprintf(_fp, "%s %-5s %s:%4d: ", buf, level_names[level], file, line);
    va_start(args, fmt);
    vfprintf(_fp, fmt, args);
    va_end(args);
    fprintf(_fp, "\n");
    fflush(_fp);
  }

  /* Release lock */
  unlock();
  
}

void Dlogger::log_dhex(int level, const char *header, char *buffer, uint16_t len, const char *file, int line) {
  if (level < _level) {
    return;
  }
  
  lock();
  
  /* Get current time */
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);
  
  FILE *outstream = _fp;
  bool is_new_line;
  int i = 2;
  while (i--) {
    /* Log to file */
    if (!outstream) {
      goto L_END;
    }
    
    if (outstream == stderr) {
      char buf[16];
      buf[strftime(buf, sizeof(buf), "[%H:%M:%S]", lt)] = '\0';
      fprintf(
        outstream, "%s %s%-5s\x1b[0m \x1b[90m%s:%4d:\x1b[0m ",
        buf, level_colors[level], level_names[level], file, line);
    }
    else {
      char buf[32];
      buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
      fprintf(_fp, "%s %-5s %s:%4d: ", buf, level_names[level], file, line);
    }
    
    fprintf(outstream, "[%s]: (%d)\n", header, len);
    fprintf(outstream, "---------------------------------------------------\n");
    
    // header line
    fprintf(outstream, "%12s 00 10 20 30 40 50 60 70 80 90 A0 B0 C0 D0 E0 F0\n", " ");
    fprintf(outstream, "%12s -----------------------------------------------\n", " ");
    
    is_new_line = true;
    for (uint16_t i = 0; i < len; i++) {
        
        if (is_new_line) {
            // header
            fprintf(outstream, "0x%08x | ", i);
            is_new_line = false;
        }
        
        fprintf(outstream, "%02x ", (uint8_t)buffer[i]);
        if (((i+1) % 0x10) == 0) {
            for (uint16_t j = i+1 - 0x10; j < i+1; j++) {
                if (isprint((int)buffer[j])) fputc(buffer[j], outstream);
                else fputc('.', outstream);
            }
            fputs("\n", outstream);
            is_new_line = true;
        }
        
        if (i == len - 1) {
            uint16_t rem = 0x10 - ((i+1) % 0x10);
            if (rem == 0x10)
                break;
            
            for (uint16_t j = 0; j < rem; j++)
              fprintf(outstream, "00 ");
            
            for (uint16_t j = i - (i % 0x10); j <= i; j++) {
                if (isprint((int)buffer[j]))
                  fputc(buffer[j], outstream);
                else
                  fputc('.', outstream);
            }
            
            for (uint16_t j = 0; j < rem; j++)
              fputc('.', outstream);
            
            fputs("\n", outstream);
        }
    }
    
    fputs("\n", outstream);
    
    L_END:
    /* Log to stderr or file */
    if (!_quiet) {
      outstream = stderr;
    }
    else {
      // quiet mode is active, so dont print to stderr
      break;
    }
  }
  
  unlock();
}

FILE *Dlogger::generate_log_file() {
    char command[64];
    char filepath[32];
    char filename[32];
    time_t now = time(NULL);
    struct tm *tm_st = localtime(&now);
    
    memset(command, 0, 64);
    memset(filepath, 0, 32);
    sprintf(filepath, "logs/%04d-%02d-%02d", tm_st->tm_year + 1900,
                                                        tm_st->tm_mon + 1,
                                                        tm_st->tm_mday);
    sprintf(command, "mkdir -p %s", filepath);
    
    // create path if doesn't exists
    system(command);
    int try_count = 0;
    do {
        memset(filename, 0, 64);
        
        sprintf(filename, "%s/server_%02d-%02d-%02d", filepath,
                                                        tm_st->tm_hour,
                                                        tm_st->tm_min,
                                                        tm_st->tm_sec);
        if (try_count > 0) {
            int len = strlen(filename);
            sprintf(&filename[len], "-%d.log", try_count);
        }
        else {
          int len = strlen(filename);
            sprintf(&filename[len], ".log");
        }
        
        try_count++;
    } while (access(filename, F_OK) != -1);
    
    return fopen(filename, "a+");
}

