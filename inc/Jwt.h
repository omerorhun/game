#include <string>

#define TOKEN_LIFETIME_SEC 1800 // 30 min

class Jwt {
    public:
    Jwt(std::string token, std::string key);
    Jwt(long user_id, long expire, std::string key);
    std::string get_token();
    bool verify();
    
    private:
    std::string _header;
    std::string _payload;
    std::string _signature;
    std::string _key;
    std::string _token;
    
    std::string sign_packet();
    std::string generate_token();
    bool encode();
    bool decode(); // parse token
    
    bool is_expired();
};
