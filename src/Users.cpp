#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "Users.h"
#include <curl/curl.h>
#include "json.hpp"
#include <fstream>
#include <sstream>

//name%2Cpicture%2Ccover%2Cage_range%2Cdevices%2Cemail%2Cgender%2Cis_verified
#define FACEBOOK_GRAPH_REQ_FIELDS "name,picture,email,gender,is_verified"
#define FACEBOOK_TOKEN_SIZE 128

using namespace std;

static size_t write_func(void *ptr, size_t size, size_t count, void *stream);
string read_file(const char *file_name);

Users *Users::p_instance = NULL;

Users::Users() {
    if (p_instance == NULL)
        p_instance = this;
}

Users *Users::get_instance() {
    return p_instance;
}

// TODO: Add all facebook errors
ErrorCodes Users::register_user(int *client_id, string access_token) {
    ErrorCodes err;
    // get user data from facebook
    cout << "waiting response from facebook..." << endl;
    nlohmann::json user_data = get_user_data_from_facebook(access_token);
    cout << "response: " << user_data.dump() << endl << endl;
    
    err = check_errors(user_data);
    if (err != ERR_SUCCESS) {
        cout << "Error code: " << err << endl;
        return err;
    }
    
    string id_str = user_data["id"];
    stringstream ss;
    ss << id_str;
    long fb_id;
    ss >> fb_id;
    
    string user_name = user_data["name"];
    string user_url = user_data["picture"]["data"]["url"];
    
    // Token verificated, then sign up or login
    // Create new session
    UserInfo user_info;
    user_info.fb_id = fb_id;
    user_info.username = user_name;
    user_info.picture_url = user_url;
    
    // check user if registered
    ifstream input_stream;
    stringstream buffer;
    
    input_stream.open("registry.json", ifstream::binary);
    if (input_stream.is_open()) {
        buffer << input_stream.rdbuf();
        input_stream.close();
    }
    
    nlohmann::json all_users_json;
    try {
         all_users_json = nlohmann::json::parse(buffer.str());
    }
    catch(nlohmann::json::parse_error e) {
        cerr << "parse error" << endl;
    }
    
    // lookup for the user
    int loc = -1;
    if ((loc = user_lookup(fb_id, all_users_json)) == -1) {
        // user not found, new registry
        nlohmann::json new_user;
        
        // TODO: Determine client's id
        user_info.id = all_users_json.size() + 10000000;
        
        new_user["id"] = user_info.id;
        new_user["fb_id"] = user_info.fb_id;
        new_user["name"] = user_info.username;
        new_user["url"] = user_info.picture_url;
        all_users_json["users"].push_back(new_user);
        
        ofstream output("registry.json", ios_base::in | ios_base::out | ios_base::trunc);
        output << all_users_json.dump();
        output.close();
        
        cout << user_info.username << " registered ["<< user_info.id << "]" << endl;
    }
    else {
        // else login
        nlohmann::json user = all_users_json["users"].at(loc);
        string name = user["name"];
        
        user_info.id = user["id"];
        string url = user["url"];
        cout << user_info.username << " logged in ["<< user_info.id << "]" << endl;
    }
    
    *client_id = user_info.id;
    return err;
}

int Users::user_lookup(long fb_id, nlohmann::json user_list) {
    int ret = -1;
    
    if (user_list.find("users") != user_list.end()) {
        for (int i = 0; i < user_list["users"].size(); i++) {
            if (user_list["users"].at(i).find("fb_id") != user_list["users"].at(i).end()) {
                nlohmann::json user = user_list["users"].at(i);
                if (user["fb_id"] == fb_id)
                    return i;
            }
        }
    }
    
    return ret;
}

nlohmann::json Users::get_user_data_from_facebook(string access_token) {
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

nlohmann::json Users::get_user_data(int uid) {
    // check user if registered
    string buffer = read_file("registry.json");
    
    nlohmann::json root = nlohmann::json::parse(buffer);
    nlohmann::json user;
    for (int i = 0; i < root["users"].size(); i++) {
        if (root["users"].at(i)["id"] == uid) {
            user = root["users"].at(i);
            break;
        }
    }
    
    return user;
}

string read_file(const char *file_name) {
    ifstream input_stream;
    stringstream buffer;
    
    input_stream.open(file_name, ifstream::binary);
    
    if (input_stream.is_open()) {
        buffer << input_stream.rdbuf();
        input_stream.close();
    }
    else {
        cerr << "File couldn't opened\n" << endl;
    }
    
    return buffer.str();
}

ErrorCodes Users::check_errors(nlohmann::json response_json) {
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