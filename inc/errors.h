#ifndef _ERRORS_H_
#define _ERRORS_H_

typedef enum{
    ERR_SUCCESS = 0,
    ERR_CONNECTION,
    ERR_REQ_LOGIN,
    ERR_REQ_WRONG_HEADER,
    ERR_REQ_WRONG_REQ_CODE,
    ERR_REQ_WRONG_LENGTH,
    ERR_REQ_CRC,
    ERR_REQ_WRONG_TOKEN,
    ERR_REQ_DISCONNECTED,
    ERR_REQ_UNKNOWN,
    ERR_USERS_LOGIN_SUCCESS,
    ERR_USERS_SIGNUP_SUCCESS,
    ERR_USERS_FB,
    ERR_FB_INVALID_ACCESS_TOKEN,
    ERR_FB_UNKNOWN,
    ERR_SRV_ACCEPT_CONN
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