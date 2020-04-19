#ifndef _REGISTRY_SERVICE_H_
#define _REGISTRY_SERVICE_H_

#include <string>

#include "json.hpp"

#include "errors.h"
#include "constants.h"
#include "GameDAL.h"

class RegistryService {
    
    public:
    RegistryService();
    
    static RegistryService *get_instance();
    
    ErrorCodes sign_in(uint64_t *uid, std::string access_token);
    bool erase_user();
    RegistryInfo get_user_data(uint64_t uid);
    nlohmann::json get_user_data_from_facebook(std::string access_token);
    ErrorCodes check_errors(nlohmann::json response_json);
    
    private:
    static RegistryService *_ps_instance;
    GameDAL *_p_game_dal;
    void parse_json_data(std::string str);
};


#endif // _REGISTRY_SERVICE_H_