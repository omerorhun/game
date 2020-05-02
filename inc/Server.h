#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <vector>
#include <netdb.h>
#include <mutex>

// for libev
#include <ev.h>

#include "errors.h"
#include "constants.h"

#define SERVER_PORT 1903

typedef struct {
    uint64_t uid;
    int socket;
}ClientConnectionInfo;

class Server {
    public:
    Server();
    Server(int port);
    
    int wait_clients();
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
    ev_io _waccept;
    static struct ev_loop *_ploop;
    static void add_new_connection(struct ev_loop *loop, ev_io *watcher, int revents);
    static void handle_client_cb(struct ev_loop *loop, ev_io *watcher, int revents);
    void handle_client(ev_io *watcher);
    
};
#endif 