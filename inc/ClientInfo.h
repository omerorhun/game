#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include <string>
#include <stdint.h>

using namespace std;

class ClientInfo {
    
    public:
    // Constructors
    ClientInfo();
    
    // Functions
    bool is_available();
    void set_username(string name);
    string get_username(void);
    void set_id(int id);
    int get_id();
    void set_fb_id(long fid);
    long get_fb_id();
    void set_picture_url(string url);
    string get_picture_url();
    void set_opponent_id(int id);
    int get_opponent_id(void);
    
    private:
    // Variables
    int id;
    int opponent_id;
    string username;
    long fb_id;
    string picture_url;
};

#endif