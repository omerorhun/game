#ifndef _SESSIONS_H_
#define _SESSIONS_H_

#define TOKEN_SIZE 128

class Sessions {
    public:
    Sessions();
    
    private:
    
    int id;
    int session_id;
    char token[TOKEN_SIZE];
};

#endif /* _SESSIONS_H_ */
