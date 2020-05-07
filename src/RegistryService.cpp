#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "RegistryService.h"
#include "utilities.h"
#include <curl/curl.h>
#include "json.hpp"
#include <fstream>
#include <sstream>
#include "debug.h"

//name%2Cpicture%2Ccover%2Cage_range%2Cdevices%2Cemail%2Cgender%2Cis_verified
#define FACEBOOK_GRAPH_REQ_FIELDS "name,picture,email,gender,is_verified"
#define FACEBOOK_TOKEN_SIZE 128

using namespace std;

static size_t write_func(void *ptr, size_t size, size_t count, void *stream);

RegistryService *RegistryService::_ps_instance = NULL;

RegistryService::RegistryService() {
    if (_ps_instance == NULL) {
        _ps_instance = this;
        
        _p_game_dal = GameDAL::get_instance();
    }
}

RegistryService *RegistryService::get_instance() {
    if (_ps_instance == NULL) {
        _ps_instance = new RegistryService();
    }
    
    return _ps_instance;
}

ErrorCodes RegistryService::sign_in(uint64_t *uid, string access_token) {
    ErrorCodes err;
    // get user data from facebook
    mlog.log_debug("waiting response from facebook...");
    nlohmann::json user_data = get_user_data_from_facebook(access_token);
    mlog.log_debug("response: %s", user_data.dump().c_str());
    err = check_errors(user_data);
    if (err != ERR_SUCCESS) {
        mlog.log_error("Error code: %d", err);
        return err;
    }
    
    // Token verificated, then sign up or login
    // Create new session
    RegistryInfo user_info;
    user_info.fb_id = stoul(string(user_data["id"]));
    user_info.name = user_data["name"];
    user_info.picture_url = user_data["picture"]["data"]["url"];
    
    if (_p_game_dal->check_registry_by_fb_id(user_info.fb_id)) {
        // login
        
        // get user from db
        RegistryInfo registry = _p_game_dal->get_user_by_fb_id(user_info.fb_id);
        
        *uid = registry.uid;
        
        mlog.log_debug("%s is already registered [%lu]", registry.name.c_str(), registry.uid);
    }
    else {
        // user not found, new registry
        // TODO: Determine client's id
        _p_game_dal->insert_user(user_info);
        
        RegistryInfo registry = _p_game_dal->get_user_by_fb_id(user_info.fb_id);
        
        // insert to statistics table
        _p_game_dal->insert_user_stat(user_info.uid);
        
        *uid = registry.uid;
        mlog.log_debug("%s registered [%lu]", user_info.name.c_str(), user_info.uid);
    }
    
    return err;
}

nlohmann::json RegistryService::get_user_data_from_facebook(string access_token) {
    CURL *hnd = curl_easy_init();
    
    curl_easy_setopt(hnd, CURLOPT_POST, 1);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://graph.facebook.com/me/");
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "content-type: application/x-www-form-urlencoded");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
    
    string fields("fields=");
    fields.append(FACEBOOK_GRAPH_REQ_FIELDS);
    string token = "&access_token=";
    token.append(access_token);
    
    string request;
    request.append(fields);
    request.append(token);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, request.c_str());
    
    string response;
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);
    
    CURLcode ret = curl_easy_perform(hnd);
    nlohmann::json user_json = nlohmann::json::parse(response);
    
    return user_json;
}

RegistryInfo RegistryService::get_user_data(uint64_t uid) {
    // check user if registered
    return _p_game_dal->get_user_by_uid(uid);
}

ErrorCodes RegistryService::check_errors(nlohmann::json response_json) {
    ErrorCodes ret = ERR_SUCCESS;
    
    if (response_json.find("error") != response_json.end()) {
        int fb_code = response_json["error"]["code"];
        ret = ERR_FB_UNKNOWN;
        if (fb_code == 190) {
            ret = ERR_FB_INVALID_ACCESS_TOKEN;
        }
    }
    
    return ret;
}

static size_t write_func(void *ptr, size_t size, size_t count, void *stream) {
    ((string*)stream)->append((char*)ptr, 0, size*count);
    return size*count;
}