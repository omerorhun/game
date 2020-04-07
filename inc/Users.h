#ifndef _USERS_H_
#define _USERS_H_

#include <string>

#include "json.hpp"

#include "errors.h"
#include "constants.h"

typedef struct {
    int id;
    int opponent_id;
    std::string username;
    long fb_id;
    std::string picture_url;
}RegisterInfo;

class Users {
    
    public:
    static Users *p_instance;
    static Users *get_instance();
    Users();
    bool login_with_facebook(int access_token);
    ErrorCodes register_user(int *client_id, std::string access_token);
    bool erase_user();
    nlohmann::json get_user_data(int client_id);
    nlohmann::json get_user_data_from_facebook(std::string access_token);
    ErrorCodes check_errors(nlohmann::json response_json);
    
    private:
    void parse_json_data(std::string str);
    int user_lookup(long fb_id, nlohmann::json user_list);
};


#endif