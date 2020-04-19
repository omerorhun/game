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
#include "debug.h"

// for libev
#if !CPP_STYLE_LIBEV
struct ev_loop *Server::_ploop = NULL;
#endif // !CPP_STYLE_LIBEV

using namespace std;

Server *Server::_ps_instance = NULL;

#if CPP_STYLE_LIBEV
Server::Server(ev::dynamic_loop &loop) {
    _ps_instance = this;

    _waccept.set(loop);
}
#else
Server::Server() {
    if (_ps_instance == NULL) {
        _ps_instance = this;
        
        // init loop
        _ploop = ev_default_loop(0);
        _port = SERVER_PORT;
    }
}

Server::Server(int port) {
    if (_ps_instance == NULL) {
        _ps_instance = this;
        
        // init loop
        _ploop = ev_default_loop(0);
        
        _port = port;
    }
}

#endif // CPP_STYLE_LIBEV

int Server::init_server() {
    int result = 0;
    
    mlog.log_debug("init server");
    
    // open a socket
    if ((_main_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        mlog.log_error("Can't create a socket!");
        return -1;
    }
    
    // bind the socket
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server_addr.sin_addr);
    server_addr.sin_port = htons(_port);
    
    if (bind(_main_socket, (const sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        mlog.log_error("Can't bind to IP/port");
        return -1;
    }
    
    // listen the socket
    if (listen(_main_socket, SOMAXCONN) == -1) {
        mlog.log_error("Can't listen!");
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
    
    mlog.log_info("init server end");
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
        mlog.log_error("Error on accept");
        close(client_socket);
        return;
    }
    
    mlog.log_debug("%s %d", __func__, client_socket);
    
    ev::io *w_client = (ev::io *)malloc(sizeof(ev::io));
    mlog.log_debug("_waccept.loop: %p", &_waccept.loop);
    w_client->set(_waccept.loop);
    w_client->set<Server, &Server::handle_client>(this);
    w_client->set(client_socket, ev::READ);
    w_client->start();
}

void Server::handle_client(ev::io &watcher, int revents) {
    Requests request(watcher.fd);
    
    mlog.log_debug("handle_client: %d", watcher.fd);
    
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
        mlog.log_error("Error on accept");
        close(client_socket);
        return;
    }
    
    print_client_status(client_addr);
    
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
        mlog.log_debug("create thread");
        *(bool *)watcher->data = false; // busy, ignore new requests
        thread *th = new thread(&Server::handle_client, Server::get_instance(), watcher);
        // TODO: release thread
    }
}

void free_watcher(ev_io *watcher) {
    mlog.log_debug("free watcher");
    
    mlog.log_debug("w1");
    free((watcher)->data);
    
    mlog.log_debug("w2");
    free(watcher);
}
thread *tmr_thr;

void Server::handle_client(ev_io *w) {
    ev_io *watcher = w;
    int sock = watcher->fd;
    Requests request(sock);
    
    mlog.log_info("handle_client: %d", sock);
    
    ErrorCodes err = request.handle_request();
    if (err == ERR_REQ_DISCONNECTED) {
        //*(bool *)watcher->data = false; // for ignoring zero byte messages
        // stop watcher
        ev_io_stop(_ploop, watcher);
        
        free_watcher(watcher);
        
        // Close socket
        close(sock);
    }
    else {
        *(bool *)watcher->data = true; // available for new requests
    }
}

int Server::wait_clients() {
    int result = -1;
    
    mlog.log_info("Waiting for clients...");
    
    // check events
    ev_run(_ploop, 0);
    
    return result;
}
#endif // CPP_STYLE_LIBEV

Server *Server::get_instance() {
    return _ps_instance;
}

vector<uint64_t> Server::get_online_clients() {
    vector<uint64_t> copy;
    for (ClientConnectionInfo i : _online_clients)
        copy.push_back(i.uid);
    
    return copy;
}

#include <algorithm>
bool Server::is_client_online(uint64_t uid) {
    ClientConnectionInfo *cci = lookup_by_uid(uid);
    if (cci == NULL)
        return false;
    
    return true;
}

mutex mtx_waiting;

int Server::login(ClientConnectionInfo client_conn) {
    
    // check if client logged in
    if (lookup_by_uid(client_conn.uid) != NULL) {
        mlog.log_debug("already logged in");
        return 0;
    }
    
    // add client
    mtx_waiting.lock();
    _online_clients.push_back(client_conn);
    mtx_waiting.unlock();
    
    return 1;
}

int Server::logout(uint64_t uid) {
    
    for (int i = 0; i < _online_clients.size(); i++) {
        if (_online_clients[i].uid == uid) {
            _online_clients.erase(_online_clients.begin() + i);
            return 1;
        }
    }
    
    return 0;
}

// TODO: add mutex here
ClientConnectionInfo *Server::lookup_by_uid(uint64_t uid) {
    ClientConnectionInfo *ret = NULL;
    for (int i = 0; i < _online_clients.size(); i++) {
        if (_online_clients[i].uid == uid) {
            ret = &_online_clients[i];
            break;
        }
    }
    
    return ret;
}

ClientConnectionInfo *Server::lookup_by_socket(int socket) {
    ClientConnectionInfo *ret = NULL;
    for (int i = 0; i < _online_clients.size(); i++) {
        if (_online_clients[i].socket == socket) {
            ret = &_online_clients[i];
            break;
        }
    }
    
    return ret;
}

int Server::get_socket(uint64_t uid) {
    ClientConnectionInfo *cci = lookup_by_uid(uid);
    
    if (cci == NULL)
        return -1;
    
    return cci->socket;
}

uint64_t Server::get_uid(int socket) {
    ClientConnectionInfo *cci = lookup_by_socket(socket);
    
    if (cci == NULL)
        return -1;
    
    return cci->uid;
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
        mlog.log_debug("%s connected to %s", host, svc);
    }
    else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        mlog.log_debug("%s connected to %hu", host, client.sin_port);
    }
    
    return;
}

// for debug
struct ev_loop *Server::get_loop() {
    return _ploop;
}
