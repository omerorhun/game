#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <vector>
#include <netdb.h>
#include <mutex>

#include "errors.h"

#define SERVER_PORT 1903

class Server {
    public:
    Server(void);
    int init_server();
    int wait_clients();
    void handle_client(int sock);
    
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
    ErrorCodes add_new_connection(fd_set *p_set);
};



#endif 