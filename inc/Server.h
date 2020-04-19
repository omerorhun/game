#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <vector>
#include <netdb.h>
#include <mutex>

#define CPP_STYLE_LIBEV 0

// for libev
#if !CPP_STYLE_LIBEV
#include <ev.h>
#else
#include <ev++.h>
#endif // !CPP_STYLE_LIBEV

#include "errors.h"
#include "constants.h"

#define SERVER_PORT 1903

typedef struct {
    uint64_t uid;
    int socket;
}ClientConnectionInfo;

class Server {
    public:
#if CPP_STYLE_LIBEV
    Server(ev::dynamic_loop &loop);
#else
    Server();
    Server(int port);
    
    int wait_clients();
#endif // CPP_STYLE_LIBEV
    int init_server();
    
    int add_client(uint64_t uid);
    int add_client_to_waiting_list(uint64_t uid);
    int login(ClientConnectionInfo client_conn);
    int logout(uint64_t uid);
    ClientConnectionInfo *lookup_by_uid(uint64_t uid);
    ClientConnectionInfo *lookup_by_socket(int socket);
    int get_socket(uint64_t uid);
    uint64_t get_uid(int socket);
    
    static Server *get_instance();
    
    std::vector<uint64_t> get_online_clients();
    bool is_client_online(uint64_t uid);
    
    static void print_client_status(sockaddr_in client);
    
    // for debug
    static struct ev_loop *get_loop();
    
    private:
    int _port;
    int _main_socket;
    static Server *_ps_instance;
    
    std::vector<ClientConnectionInfo> _online_clients;
    
    // for libev
#if !CPP_STYLE_LIBEV
    ev_io _waccept;
    static struct ev_loop *_ploop;
    static void add_new_connection(struct ev_loop *loop, ev_io *watcher, int revents);
    static void handle_client_cb(struct ev_loop *loop, ev_io *watcher, int revents);
    void handle_client(ev_io *watcher);
#else
    ev::io _waccept;
    void add_new_connection(ev::io &watcher, int revents);
    void handle_client(ev::io &watcher, int revents);
#endif // !CPP_STYLE_LIBEV
    
};
#endif 