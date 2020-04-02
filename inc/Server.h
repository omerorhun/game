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
#endif

#include "errors.h"

#define SERVER_PORT 1903

class Server {
    public:
#if CPP_STYLE_LIBEV
    Server(ev::dynamic_loop &loop);
#else
    Server();
    int wait_clients();
#endif
    int init_server();
    
    int add_client(int uid);
    int add_client_to_waiting_list(int uid);
    int login(int uid);
    int logout(int uid);
    bool lookup(int uid, std::vector<int>::iterator *it);
    
    static Server *get_instance();
    
    void add_message_by_id(int id, std::string msg);
    std::string get_message_by_id(int id);
    int check_for_messages(int id);
    
    std::vector<int> get_online_clients();
    bool is_client_online(int id);
    
    void print_client_status(sockaddr_in client);
    
    private:
    int _main_socket;
    static Server *p_instance;
    void add_messagebox(int id);
    
    std::vector<int> online_clients;
    std::vector<int> waiting_clients;
    std::vector<int> active_clients;
    std::map<int, std::vector<std::string> > message_queue;
    
    // for select()
    fd_set _ready_sockets;
    int _clients[SOMAXCONN];
    std::mutex mtx_clients[SOMAXCONN];
    int client_count;
    //ErrorCodes add_new_connection(fd_set *p_set);
    
    // for libev
#if !CPP_STYLE_LIBEV
    ev_io _waccept;
    static struct ev_loop *_ploop;
    static void add_new_connection(struct ev_loop *loop, ev_io *watcher, int revents);
    static void handle_client(struct ev_loop *loop, ev_io *watcher, int revents);
#else
    ev::io _waccept;
    void add_new_connection(ev::io &watcher, int revents);
    void handle_client(ev::io &watcher, int revents);
#endif
    
};
#endif 