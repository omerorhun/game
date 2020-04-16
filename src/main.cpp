/*
    Server 23.02.2020
    
    TODO:
    - requests will be limited by per second
    - there will be a restriction to requests per second from same IP
    - threads will be pre-allocated
    - logs must be printed while on idle process. there must be idle process.
*/

// standart libs
#include <iostream>
#include <stdio.h>

// for special ops
#include "Server.h"
#include "Users.h"
#include "Matcher.h"
#include "debug.h"
#include "version.h"

using namespace std;

Dlogger mlog;

bool g_is_active = true;

bool get_args(int argc, char **argv, uint16_t *port);

int main (int argc, char **argv) {
    uint16_t port;
    
    mlog.log_info("Version: %s", VERSION_FILEVERSION_STR);
    
    get_args(argc, argv, &port);
    
#if CPP_STYLE_LIBEV
    ev::dynamic_loop loop;
    Server server(loop);
#else
    Server server(port);
#endif
    Users users;
    Matcher matcher;
    
    if (server.init_server() == -1)
        return -1;

#if CPP_STYLE_LIBEV    
    mlog.log_info("Waiting for clients...");
    loop.run(0);
#else
    server.wait_clients();
#endif
    
    return 0;
}

bool get_args(int argc, char **argv, uint16_t *port) {
    
    // set default values
    *port = SERVER_PORT;
    
    // check token, host, port, userid
    for (int i = 0; i < argc; i++) {
        
        char *ptr = strstr(argv[i], "=");
        ptr++;
        
        if (strstr(argv[i], "port=")) {
            // get port no
            uint16_t temp = atoi(ptr);
            if (isdigit(ptr[0])) {
                *port = temp;
            }
        }
        else {
            // do nothing
        }
    }
    
    return true;
}