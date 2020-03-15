#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include <string>
#include <stdint.h>

using namespace std;

#define RX_BUFFER_SIZE 1024

class ClientInfo {
    
    public:
    // Constructors
    ClientInfo();
    ClientInfo(int sock);
    
    // Functions
    bool is_available();
    void set_username(string name);
    string get_username(void);
    void set_socket(int sock);
    int get_socket();
    void set_id(int id);
    int get_id();
    void set_session_id(int);
    int get_session_id();
    void set_fb_id(long fid);
    long get_fb_id();
    void set_picture_url(string url);
    string get_picture_url();
    
    void set_opponent_id(int id);
    int get_opponent_id(void);
    
    int listen(void);
    void message_handler(void);
    int interprete_message(string msg);
    
    void login();
    void logout();
    
    
    typedef enum {
        IDLE = 0,
        GET_ONLINE_USERS,
        SEND_MATCH_REQUEST,
        GET_MATCH_REQUEST,
        SEND_RESPONSE_TO_MATCH_REQUEST,
        GET_RESPONSE_TO_MATCH_REQUEST,
        RESPOND_MATCH_REQUEST,
        GET_MESSAGE_REQUEST,
        SEND_MESSAGE_REQUEST,
        DISCONNECT_REQUEST,
        OPPONENT_DISCONNECTED,
        SIGN_IN_REQUEST_RECEIVED,
        LOGOUT_REQUEST,
        TEST
    }RequestTypes;
    
    private:
    void listen_user(void);
    
    // Variables
    static int client_count;
    int id;
    int opponent_id;
    int session_id;
    string username;
    uint8_t password;
    int socket;
    
    long fb_id;
    string picture_url;
    
    bool availability;
};

#endif