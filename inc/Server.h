#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <vector>

#define SERVER_PORT 1903

using namespace std;

#include <netdb.h>

class Server {
    public:
    Server(void);
    int init_server();
    int wait_clients();
    void handle_client(int sock);
    
    void create_session(int client_id);
    int add_client(int client_id);
    int add_client_to_waiting_list(int client_id);
    int login(int client_id);
    int logout(int client_id);
    
    static Server *get_instance();
    
    void add_message_by_id(int id, string msg);
    string get_message_by_id(int id);
    int check_for_messages(int id);
    
    vector<int> get_online_clients();
    bool is_client_online(int id);
    
    void print_client_status(sockaddr_in client);
    
    private:
    int main_socket;
    static Server *p_instance;
    
    void add_messagebox(int id);
    
    vector<int> online_clients;
    vector<int> waiting_clients;
    vector<int> active_clients;
    map<int, vector<string> > message_queue;
};



#endif 