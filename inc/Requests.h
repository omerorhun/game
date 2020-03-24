#ifndef _REQUESTS_H_
#define _REQUESTS_H_

#include "Protocol.h"

#define RX_BUFFER_SIZE 128
#define TX_BUFFER_SIZE 128
#define TOKEN_SIZE 128

#define REQUEST_HEADER 0x01

const std::string jwt_key = "bjk1903";

typedef enum{
    ERR_REQ_SUCCESS = 0,
    ERR_REQ_WRONG_HEADER,
    ERR_REQ_WRONG_REQ_CODE,
    ERR_REQ_WRONG_LENGTH,
    ERR_REQ_CRC,
    ERR_REQ_UNKNOWN
}RequestErrorCodes;

typedef enum {
    REQ_LOGIN,
    REQ_FB_LOGIN,
    REQ_FB_LOGIN_SUCCES,
    REQ_FB_LOGIN_FAIL,
    REQ_COUNT,
    REQ_LOGOUT,
    REQ_MATCH,
    REQ_GET_ONLINE_USERS,
    REQ_GET_ONLINE_USERS_SUCCESS,
    REQ_DISCONNECT,
    REQ_ERROR,
    
#if 0    
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
#endif
}RequestCodes;

static const int lengths[][2] = 
{
    {REQ_LOGIN, 0}, 
    {REQ_GET_ONLINE_USERS, 0},
};

#define GET_ONLINE_USERS_LEN 0


class Requests {
  
  public:

  Requests(int sock);
  void get_request();
  RequestErrorCodes check_request();
  void handle_request();
  void send_response();
  
  void login(int client_id);
  
  
  private:
  Protocol _in_packet;
  Protocol _out_packet;
  
  int socket;
  
  char token[TOKEN_SIZE];
  RequestCodes req_code;
  char rx_buffer[RX_BUFFER_SIZE];
  char tx_buffer[TX_BUFFER_SIZE];
  
  bool check_request_code();
  bool check_length();
  void prepare_error_packet(RequestErrorCodes err);
};



#endif /* _REQUESTS_H_*/