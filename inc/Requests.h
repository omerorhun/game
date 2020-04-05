#ifndef _REQUESTS_H_
#define _REQUESTS_H_

#include "Protocol.h"
#include "errors.h"
#include "constants.h"
#include <ev.h>

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
    REQ_CANCEL_MATCH,
    REQ_START_GAME,
}RequestCodes;

class Requests {
  
  public:
  Requests(int sock);
  ~Requests();
  
  ErrorCodes handle_request();
  ErrorCodes get_request();
  ErrorCodes check_request(int *p_uid);
  ErrorCodes interpret_request(int uid, RequestCodes req_code, std::string indata);
  void prepare_error_packet(ErrorCodes err);
  void send_response();
  
  void login(int client_id);
  void logout(int client_id);
  
  // for libev
  struct ev_loop *_event_listener;
  
  private:
  Protocol _in_packet;
  Protocol _out_packet;
  
  int socket;
  RequestCodes req_code;
  
  bool set_header(uint8_t header);
  bool set_request_code(RequestCodes req_code);
  bool set_ack(uint8_t ack);
  bool set_token(int id);
  bool add_data(std::string data);
  bool add_data(uint8_t *data, uint16_t len);
  
  bool check_request_code();
  bool check_length();
  
};

#endif /* _REQUESTS_H_*/