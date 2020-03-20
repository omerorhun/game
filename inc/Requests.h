#ifndef _REQUESTS_H_
#define _REQUESTS_H_

#define RX_BUFFER_SIZE 128
#define TX_BUFFER_SIZE 128
#define TOKEN_SIZE 128

#define REQUEST_HEADER 0x01

#define B0(x)               ((x) & 0xFF)
#define B1(x)               (((x)>>8) & 0xFF)
#define B2(x)               (((x)>>16) & 0xFF)
#define B3(x)               (((x)>>24) & 0xFF)

#define GET_LENGTH(p) (uint16_t)(((p[1] << 0) & 0xff) + \
                                ((p[2] << 8) & 0xff00))

#define GET_REQUEST_CODE(p) (RequestCodes)(p[3])

typedef enum{
    ERR_REQ_SUCCESS = 0,
    ERR_REQ_WRONG_HEADER,
    ERR_REQ_WRONG_REQ_CODE,
    ERR_REQ_WRONG_LENGTH,
    ERR_REQ_UNKNOWN
}RequestErrorCodes;

typedef enum {
    REQ_LOGIN,
    REQ_FB_LOGIN,
    REQ_COUNT,
    REQ_LOGOUT,
    REQ_MATCH,
    REQ_GET_ONLINE_USERS,
    REQ_DISCONNECT
    
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
class Requests {
  
  public:

  Requests(int sock);
  void get_request();
  RequestErrorCodes check_request();
  void handle_request();
  void send_response();
  
  void login(int client_id);
  
  
  private:
  int socket;
  
  uint16_t length;
  
  char fb_token[128];
  char token[TOKEN_SIZE];
  RequestCodes req_code;
  char *data;
  char rx_buffer[RX_BUFFER_SIZE];
  char tx_buffer[TX_BUFFER_SIZE];
  
  bool check_request_code();
  bool check_length();
};



#endif /* _REQUESTS_H_*/