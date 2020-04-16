#include <iostream>
#include <openssl/hmac.h>
#include <time.h>
#include "base64.h"
#include "json.hpp"

#include "Jwt.h"
#include "utilities.h"
#include "debug.h"

using namespace std;

Jwt::Jwt(string token, string key) {
    _key = key;
    _token = token;
    _is_valid = false;
    if (this->decode())
        _is_valid = true;
}

Jwt::Jwt(long user_id, long expire, string key) {
    nlohmann::json header = {{"alg", "HS256"},{"typ", "JWT"}};
    
    _header.append(header.dump());
    
    nlohmann::json payload_json;
    payload_json["id"] = user_id;
    payload_json["expire"] = expire;
    
    _payload = payload_json.dump();
    _key = key;
    
    generate_token();
    
    _is_valid = true;
}

string Jwt::get_token() {
    return _token;
}

string Jwt::generate_token() {
    _signature = sign_packet();
    
    _token.append(base64_encode((const uint8_t *)_header.c_str(), _header.size()));
    _token.append(".");
    _token.append(base64_encode((const uint8_t *)_payload.c_str(), _payload.size()));
    _token.append(".");
    _token.append(base64_encode((const uint8_t *)_signature.c_str(), _signature.size()));
    
    return _token;
}

string Jwt::sign_packet() {
    string header = _header + '.' + _payload;
    HMAC_CTX *ctx = HMAC_CTX_new();
    
    const EVP_MD *md = EVP_sha256();
    uint32_t key_size = EVP_MD_size(md);
    uint8_t signature[key_size];
    
    HMAC_Init_ex(ctx, _key.c_str(), _key.size(), md, NULL);
    HMAC_Update(ctx, (const unsigned char *)header.c_str(), header.size());
    HMAC_Final(ctx, signature, &key_size);
    
    return string((char *)signature, key_size);
}

// TODO: adjust this function. remove strstr funcs.
bool Jwt::decode() {
    _token.append("\r");
    char *tkn = (char *)_token.c_str();
    
    char *tail = tkn;
    char *head = strstr((char *)tkn, ".");
        
    uint8_t cnt = 0;
    do {
        if (head == NULL)
            return false;
        
        uint16_t size = (head-tail);
        char first[size];
        for (uint16_t i = 0; i < size; i++) {
            first[i] = tail[i];
        }
        
        if (cnt == 0) {
            string temp = base64_decode(string(first, size));
            if (!nlohmann::json::accept(temp))
                return false;
            
            _header = temp;
        }
        else if (cnt == 1) {
            string temp = base64_decode(string(first, size));
            if (!nlohmann::json::accept(temp))
                return false;
            
            _payload = temp;
        } 
        else {
            _signature = base64_decode(string(first, size));
            break;
        }
        
        tail = head + 1;
        
        cnt++;
    } while((head = strstr(tail, ".")) || (head = strstr(tail, "\r")));
    
    return true;
} 

bool Jwt::verify() {
    if (!_is_valid)
        return false;
    
    if (is_expired())
        return false;
    
    string signature = sign_packet();
    if (_signature.compare(signature) == 0)
        return true;
    
    return false;
}

bool Jwt::is_expired() {
    if (!_is_valid)
        return false;
    
    nlohmann::json payload_json = nlohmann::json::parse(_payload);
    time_t expire = payload_json["expire"];
    time_t now; // TODO: get time in UTC
    time(&now);
    
    mlog.log_debug("%d minutes to expire", (int)(TOKEN_LIFETIME_SEC - (now - expire))/60);
    
    if ((now - expire) < TOKEN_LIFETIME_SEC)
        return false;
    
    return true;
}

int Jwt::get_uid() {
    nlohmann::json plaod = nlohmann::json::parse(_payload);
    int uid = plaod["id"];
    return uid;
}
