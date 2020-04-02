#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <thread>
#include <mutex>

#include "Server.h"
#include "Requests.h"

// for libev
#if !CPP_STYLE_LIBEV
#include <ev.h>
struct ev_loop *Server::_ploop = NULL;
#else
#include <ev++.h>
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
    p_instance = this;
    
    // init loop
    _ploop = ev_default_loop(0);
}
#endif // CPP_STYLE_LIBEV

int Server::init_server() {
    int result = -1;
    
    // open a socket
    if ((_main_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "Can't create a socket!";
        return result;
    }
    
    // bind the socket
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (bind(_main_socket, (const sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        cerr << "Can't bind to IP/port" << endl;
        return result;
    }
    
    // listen the socket
    if ((result = listen(_main_socket, SOMAXCONN)) == -1) {
        cerr << "Can't listen!" << endl;
        return result;
    }
    
#if !CPP_STYLE_LIBEV
    _waccept.fd = _main_socket;
    ev_io_init(&_waccept, add_new_connection, _main_socket, EV_READ);
    ev_io_start(_ploop, &_waccept);
#else
    _waccept.set<Server, &Server::add_new_connection>(this);
    _waccept.set(_main_socket, ev::READ);
    _waccept.start();
#endif    
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
        cerr << "Error on accept\n";
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
        cerr << "Error on accept\n";
        close(client_socket);
        return;
    }
    
    // add event listener for this client
    ev_io *w_client = (ev_io *)malloc(sizeof(ev_io));
    w_client->fd = client_socket;
    ev_io_init(w_client, handle_client, client_socket, EV_READ);
    ev_io_start(_ploop, w_client);
}

#include <string.h>
void Server::handle_client(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    Requests request(watcher->fd);
    
    printf("handle_client: %d\n", watcher->fd);
    
    ErrorCodes err = request.handle_request();
    if (err == ERR_REQ_DISCONNECTED) {
        // Close socket
        close(watcher->fd);
        
        // stop io
        ev_io_stop(loop, watcher);
        free(watcher);
    }
}

int Server::wait_clients() {
    int result = -1;
    
    cout << "Waiting for clients.." << endl;
    
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
    
    cout << "add messagebox for " << sesion_id << endl;
    
    mtx.lock();
    message_queue.insert(pair<int,vector<string> >(sesion_id, vector<string>()));
    mtx.unlock();
    
    if (size == message_queue.size())
        cout << "error on adding user\n";
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
        cout << "queue size: " << message_queue.size() << endl;
        cout << "id: " << session_id << endl;
        cout << "out of range " << oor.what() << endl;
        cout << "-------------------------------------------------------------------" << endl;
    }
    
    return size;
}

vector<int> Server::get_online_clients() {
    return online_clients;
}

#include <algorithm>

bool Server::is_client_online(int uid) {
    auto it = find(online_clients.begin(), online_clients.end(), uid);
    if (it == online_clients.end())
        return false;
    
    return true;
}

mutex mtx_waiting;

int Server::login(int uid) {
    vector<int>::iterator it;
    
    // check if client logged in
    if (lookup(uid, &it)) {
        printf("already logged in\n");
        return 0;
    }
    
    // add client
    mtx_waiting.lock();
    online_clients.push_back(uid);
    mtx_waiting.unlock();
    
    return 1;
}

int Server::logout(int uid) {
    vector<int>::iterator it;
    if (!lookup(uid, &it))
        return 0;
    
    online_clients.erase(it);
    
    return 1;
}

bool Server::lookup(int uid, vector<int>::iterator *it) {
    *it = find(online_clients.begin(), online_clients.end(), uid);
    if (*it == online_clients.end())
        return false;
    
    return true;
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
        cout << string(host) << " connected to " << string(svc) << endl;
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << string(host) << " connected to " << ntohs(client.sin_port) << endl;
    }
    
    return;
}
