#ifndef _REQUESTS_H_
#define _REQUESTS_H_

#include "Protocol.h"
#include "Matcher.h"
#include "Server.h"
#include "Game.h"
#include "errors.h"
#include "constants.h"
#include "json.hpp"

#include <ev.h>
#include <vector>
#include <queue>
#include <list>

#define REQUEST_HEADER 0x01
#define ACK 0x01
#define NACK 0x02
#define GET_ONLINE_USERS_LEN 0
#define MATCH_TIMEOUT 20

const std::string jwt_key = "bjk1903";

typedef struct {
  int socket;
  Protocol packet;
}NotificationInfo;

class Requests {
  
  public:
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
      REQ_GAME_START,
      REQ_GAME_ACCEPTED,
      REQ_GAME_ANSWER,
      REQ_GAME_OPPONENT_ANSWER,
      REQ_GAME_QUESTION_COMPLETED,
      REQ_GAME_RESIGN,
      REQ_GAME_OPPONENT_RESIGNED,
      REQ_GAME_OPPONENT_TIMEOUT,
      REQ_GAME_FINISH,
      REQ_GAME_NOT_ACCEPTED
  }RequestCodes;
  
  Requests(int sock);
  ~Requests();
  
  ErrorCodes handle_request();
  ErrorCodes get_request();
  ErrorCodes check_request(uint64_t *p_uid);
  ErrorCodes interpret_request(uint64_t uid, RequestCodes req_code, std::string indata);
  void prepare_error_packet(ErrorCodes err);
  void send_response();
  
  void add_notification(int socket, RequestCodes req_code, std::string data);
  static void send_notification_async(int socket, RequestCodes req_code, std::string data);
  
  void login(ClientConnectionInfo client_conn);
  void logout(uint64_t uid);
  
  // for libev
  struct ev_loop *_event_listener;
  
  private:
  Protocol _in_packet;
  Protocol _out_packet;
  std::queue<NotificationInfo> _notifications;
  
  int _uid;
  int socket;
  RequestCodes req_code;
  
  // protocol
  bool set_header(uint8_t header);
  bool set_request_code(RequestCodes req_code);
  bool set_ack(uint8_t ack);
  bool set_token(uint64_t uid);
  bool add_data(std::string data);
  bool add_data(uint8_t *data, uint16_t len);
  
  // match
  ErrorCodes add_to_match_queue(UserMatchInfo *user);
  void remove_from_match_queue(UserMatchInfo *user);
  void cancel_match(uint64_t uid, int game_id);
  
  // game
  Game *create_game(Rivals rivals);
  int get_socket(uint64_t op_uid);
  uint64_t get_uid(int socket);
  int get_game_id(uint64_t uid);
  ErrorCodes get_game_answer(std::string data, std::string &answer);
  static ErrorCodes check_game_request(RequestCodes req_code, std::string answer);
  static bool check_result_json(nlohmann::json results);
  
  bool check_request_code();
  bool check_length();
  
};

#endif /* _REQUESTS_H_*/