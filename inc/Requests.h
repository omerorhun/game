#ifndef _REQUESTS_H_
#define _REQUESTS_H_

#include "Protocol.h"
#include "errors.h"

#define REQUEST_HEADER 0x01
#define ACK 0x01
#define NACK 0x02

#define GET_ONLINE_USERS_LEN 0

const std::string jwt_key = "bjk1903";

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
  
  void handle_request();
  ErrorInfo get_request(RequestCodes *p_req_code, std::string *p_indata);
  ErrorInfo check_request();
  ErrorInfo interpret_request(RequestCodes req_code, std::string indata);
  void prepare_error_packet(ErrorInfo err);
  void send_response();
  
  void login(int client_id);
  
  private:
  Protocol _in_packet;
  Protocol _out_packet;
  
  int socket;
  RequestCodes req_code;
  
  bool check_request_code();
  bool check_length();
  
};

#endif /* _REQUESTS_H_*/