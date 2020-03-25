#ifndef _REQUESTS_H_
#define _REQUESTS_H_

#include "Protocol.h"

#define TOKEN_SIZE 128
#define REQUEST_HEADER 0x01
#define ACK 0x01
#define NACK 0x02

#define GET_ONLINE_USERS_LEN 0

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
    REQ_COUNT,
    REQ_LOGOUT,
    REQ_MATCH,
    REQ_GET_ONLINE_USERS,
    REQ_DISCONNECT,
    REQ_ERROR,
}RequestCodes;


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
  RequestCodes req_code;
  
  bool check_request_code();
  bool check_length();
  void prepare_error_packet(RequestErrorCodes err);
};

#endif /* _REQUESTS_H_*/