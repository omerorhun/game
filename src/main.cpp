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

using namespace std;

bool g_is_active = true;

int main () {
#if CPP_STYLE_LIBEV
    ev::dynamic_loop loop;
    Server server(loop);
#else
    Server server;
#endif
    Users users;
    Matcher matcher;
    
    if (server.init_server() == -1)
        return -1;

#if CPP_STYLE_LIBEV    
    printf("Waiting for clients...\n");
    loop.run(0);
#else
    server.wait_clients();
#endif
    
    return 0;
}

