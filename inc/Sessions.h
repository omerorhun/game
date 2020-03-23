#ifndef _SESSIONS_H_
#define _SESSIONS_H_

#define TOKEN_SIZE 128

class Sessions {
    public:
    Sessions();
    
    void start();
    void stop();
    void refresh_token();
    void get_token(char *p_tkn);
    bool check_token(); // check if token has timed out
    
    private:
    
    int id;
    int session_id;
    char token[TOKEN_SIZE];
    int start_dt_utc;
    void create_token(char *p_tkn);
    
};

#endif /* _SESSIONS_H_ */
