/*
    TODO:
    - requests will be limited by per second
    - there will be a restriction to requests per second from same IP
    - threads will be pre-allocated
*/

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <fcntl.h>

#include <thread>
#include <mutex>

#include "Server.h"
#include "Requests.h"

// for libev
#if !CPP_STYLE_LIBEV
struct ev_loop *Server::_ploop = NULL;
#endif // !CPP_STYLE_LIBEV

using namespace std;

Server *Server::p_instance = NULL;

#if CPP_STYLE_LIBEV
Server::Server(ev::dynamic_loop &loop) {
    p_instance = this;

    _waccept.set(loop);
}
#else
Server::Server() {
    if (p_instance == NULL)
        p_instance = this;
    
    // init loop
    _ploop = ev_default_loop(0);
}
#endif // CPP_STYLE_LIBEV

int Server::init_server() {
    int result = 0;
    
    printf("init server\n");
    
    // open a socket
    if ((_main_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Can't create a socket!\n");
        return -1;
    }
    
    // bind the socket
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (bind(_main_socket, (const sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("Can't bind to IP/port\n");
        return -1;
    }
    
    // listen the socket
    if (listen(_main_socket, SOMAXCONN) == -1) {
        printf("Can't listen!\n");
        return -1;
    }
    
#if !CPP_STYLE_LIBEV
    _waccept.fd = _main_socket;
    ev_io_init(&_waccept, add_new_connection, _main_socket, EV_READ);
    ev_io_start(_ploop, &_waccept);
#else
    _waccept.set<Server, &Server::add_new_connection>(this);
    _waccept.set(_main_socket, ev::READ);
    _waccept.start();
#endif // !CPP_STYLE_LIBEV


    printf("init server end\n");
    return result;
}

#if CPP_STYLE_LIBEV
// cpp style libev
void Server::add_new_connection(ev::io &watcher, int revents) {
    int client_socket;
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    if ((client_socket = accept(watcher.fd, (sockaddr *)&client_addr, &client_len)) == -1) {
        // acception error
        printf("Error on accept\n");
        close(client_socket);
        return;
    }
    
    printf("%s %d\n", __func__, client_socket);
    
    ev::io *w_client = (ev::io *)malloc(sizeof(ev::io));
    printf("_waccept.loop: %p\n", &_waccept.loop);
    w_client->set(_waccept.loop);
    w_client->set<Server, &Server::handle_client>(this);
    w_client->set(client_socket, ev::READ);
    w_client->start();
}

void Server::handle_client(ev::io &watcher, int revents) {
    Requests request(watcher.fd);
    
    printf("handle_client: %d\n", watcher.fd);
    
    ErrorCodes err = request.handle_request();
    if (err == ERR_REQ_DISCONNECTED) {
        // Close socket
        close(watcher.fd);
        
        // stop io
        watcher.stop();
        free(&watcher);
    }
}
#else

void Server::add_new_connection(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    int client_socket;
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    if ((client_socket = accept(watcher->fd, (sockaddr *)&client_addr, &client_len)) == -1) {
        // acception error
        printf("Error on accept\n");
        close(client_socket);
        return;
    }
    
    // add event listener for this client
    ev_io *w_client = (ev_io *)malloc(sizeof(ev_io)); // TEST: release this
    w_client->fd = client_socket;
    w_client->data = (void *)malloc(sizeof(bool)); // TEST: release this
    *(bool*)w_client->data = true; // available for new requests
    ev_io_init(w_client, handle_client_cb, client_socket, EV_READ);
    ev_io_start(_ploop, w_client);
}

#include <string.h>
void Server::handle_client_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    // TEST: when client disconnects, receiving a lot of zero byte messages. must not be created new thread that situation.
    
    if (*((bool *)watcher->data) == true){
        *(bool *)watcher->data = false; // busy, ignore new requests
        thread *th = new thread(&Server::handle_client, Server::get_instance(), &watcher);
        // TODO: release thread
    }
}

void free_watcher(ev_io **watcher) {
    printf("free watcher\n");
    if ((*watcher)->data != NULL) {
        printf("w1\n");
        free((*watcher)->data);
        (*watcher)->data = NULL;
    }
    
    if (*watcher != NULL) {
        printf("w2\n");
        free(*watcher);
        *watcher = NULL;
    }
}

void Server::handle_client(ev_io **w) {
    ev_io *watcher = *w;
    int sock = watcher->fd;
    Requests request(sock);
    
    printf("handle_client: %d\n", sock);
    
    ErrorCodes err = request.handle_request();
    if (err == ERR_REQ_DISCONNECTED) {
        //*(bool *)watcher->data = false; // for ignoring zero byte messages
        // stop watcher
        ev_io_stop(_ploop, watcher);
        
        free_watcher(w);
        
        //free(watcher->data); // TEST: may cause segmentaion fault? (no this is safe)
        //free(watcher); // TEST: may cause segmentaion fault? (no this is safe)
        
        // Close socket
        close(sock);
    }
    else {
        *(bool *)watcher->data = true; // available for new requests
    }
}



int Server::wait_clients() {
    int result = -1;
    
    printf("Waiting for clients...\n");
    
    // check events
    ev_run(_ploop, 0);
    
    return result;
}
#endif // CPP_STYLE_LIBEV

Server *Server::get_instance() {
    return p_instance;
}

mutex mtx;

void Server::add_messagebox(int sesion_id) {
    vector<string> subqueue;
    int size = message_queue.size();
    
    printf("add messagebox for %d\n", sesion_id);
    
    mtx.lock();
    message_queue.insert(pair<int,vector<string> >(sesion_id, vector<string>()));
    mtx.unlock();
    
    if (size == message_queue.size())
        printf("error on adding user\n");
}

void Server::add_message_by_id(int sesion_id, string msg) {
    if (message_queue.find(sesion_id) == message_queue.end()) {
        return;
    }
    
    message_queue.at(sesion_id).push_back(msg);
}

string Server::get_message_by_id(int sesion_id) {
    string msg("empty");
    
    if (message_queue.find(sesion_id) == message_queue.end()) {
        return msg;
    }
    
    vector<string> *p_user = &message_queue.at(sesion_id);
    
    if (message_queue.at(sesion_id).size() > 0) {
        msg = (*p_user).front();
        p_user->erase(p_user->begin());
    }
    
    return msg;
}

int Server::check_for_messages(int session_id) {
    int size = 0;
    
    try {
        mtx.lock();
        size = message_queue.at(session_id).size();
        mtx.unlock();
        
        if (size > 0)
            printf("%d messages for user%d\n", size, session_id);
    }
    catch (const out_of_range& oor) {
        printf("queue size: %d\n", (int)message_queue.size());
        printf("id: %d\n", session_id);
        printf("out of range %s\n", oor.what());
        printf("-------------------------------------------------------------------\n");
    }
    
    return size;
}

vector<int> Server::get_online_clients() {
    vector<int> copy;
    for (ClientConnectionInfo i : _online_clients)
        copy.push_back(i.uid);
    
    return copy;
}

#include <algorithm>
bool Server::is_client_online(int uid) {
    ClientConnectionInfo *cci = lookup(uid);
    if (cci == NULL)
        return false;
    
    return true;
}

mutex mtx_waiting;

int Server::login(ClientConnectionInfo client_conn) {
    
    // check if client logged in
    if (lookup(client_conn.uid) != NULL) {
        printf("already logged in\n");
        return 0;
    }
    
    // add client
    mtx_waiting.lock();
    _online_clients.push_back(client_conn);
    mtx_waiting.unlock();
    
    return 1;
}

int Server::logout(int uid) {
    
    for (int i = 0; i < _online_clients.size(); i++) {
        if (_online_clients[i].uid == uid) {
            _online_clients.erase(_online_clients.begin() + i);
            return 1;
        }
    }
    
    return 0;
}

// TODO: add mutex here
ClientConnectionInfo *Server::lookup(int uid) {
    ClientConnectionInfo *ret = NULL;
    for (int i = 0; i < _online_clients.size(); i++) {
        if (_online_clients[i].uid == uid) {
            ret = &_online_clients[i];
            break;
        }
    }
    
    return ret;
}

int Server::get_socket(int uid) {
    ClientConnectionInfo *cci = lookup(uid);
    
    if (cci == NULL)
        return -1;
    
    return cci->socket;
}

#include <string.h>
void Server::print_client_status(sockaddr_in client) {
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];
    
    memset(&host, 0, NI_MAXHOST);
    memset(&svc, 0, NI_MAXSERV);
    
    int result = getnameinfo((const sockaddr *)&client, sizeof(client), 
                                            host, NI_MAXHOST, svc, NI_MAXSERV, 0);
    
    if (result) {
        printf("%s connected to %s\n", host, svc);
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        printf("%s connected to %hu\n", host, client.sin_port);
    }
    
    return;
}
