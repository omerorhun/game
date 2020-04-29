#ifndef _QUESTIONS_H_
#define _QUESTIONS_H_

#include <string>

#define QUESTIONS_SOURCE_FILE "questions.json"

class Questions {
    
    public:
    
    static Questions *get_instance();
    
    Questions();
    std::string get_question(int count, uint8_t type);
    
    private:
    static Questions *_ps_instance;
    
};

#endif // _QUESTIONS_H_