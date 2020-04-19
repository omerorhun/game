#ifndef _ERRORS_H_
#define _ERRORS_H_

#include "constants.h"

typedef enum{
    ERR_SUCCESS = 0,
    ERR_CONNECTION,                 // 1
    ERR_REQ_LOGIN,                  // 2
    ERR_REQ_WRONG_HEADER,           // 3
    ERR_REQ_WRONG_REQ_CODE,         // 4
    ERR_REQ_WRONG_LENGTH,           // 5
    ERR_REQ_CRC,                    // 6
    ERR_REQ_WRONG_TOKEN,            // 7
    ERR_REQ_DISCONNECTED,           // 8
    ERR_REQ_MATCH_FAIL,             // 9
    ERR_REQ_UNKNOWN,                // 10
    ERR_USERS_LOGIN_SUCCESS,        // 11
    ERR_USERS_SIGNUP_SUCCESS,       // 12
    ERR_USERS_FB,                   // 13
    ERR_FB_INVALID_ACCESS_TOKEN,    // 14
    ERR_FB_UNKNOWN,                 // 15
    ERR_SRV_ACCEPT_CONN,            // 16
    ERR_GAME_NOT_MATCHED,           // 17
    ERR_GAME_START_FAIL,            // 18
    ERR_GAME_START_TIMEOUT,         // 19
    ERR_GAME_NOT_FOUND,             // 20
    ERR_GAME_WRONG_PACKET,          // 21
    ERR_LOGIN_ALREADY_LOGGED_IN     // 22
}ErrorCodes;

typedef enum {
    ERR_SUB_SUCCESS,
    ERR_SUB_UNKNOWN
}ErrorSubCodes;

typedef struct {
    ErrorCodes code;
    ErrorSubCodes subcode;
}ErrorInfo;

#endif // _ERRORS_H_