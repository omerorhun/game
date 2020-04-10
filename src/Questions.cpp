#include <stdio.h>

#include "Questions.h"
#include "utilities.h"
#include "json.hpp"

using namespace std;

Questions *Questions::_p_instance = NULL;

Questions::Questions() {
    if (_p_instance == NULL)
        _p_instance = this;
}

Questions *Questions::get_instance() {
    if (_p_instance == NULL)
        _p_instance = new Questions();
    
    return _p_instance;
}

string Questions::get_question(int count) {
    nlohmann::json ret;
    
    string buffer = read_file(QUESTIONS_SOURCE_FILE);
    
    printf("buffer file: %s\n", buffer.c_str());
    
    if (nlohmann::json::accept(buffer) == false)
        return string("");
    
    nlohmann::json questions = nlohmann::json::parse(buffer);
    
    for (int i = 0; i < count; i++) {
        
        // pick random question
        int idx = rand()%questions["questions"].size();
        
        // add to return json object
        ret["questions"].push_back(questions["questions"].at(idx));
        
        // delete from copy
        questions["questions"].erase(idx);
    }
    
    return ret.dump();
}