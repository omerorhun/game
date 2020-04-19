#include <stdio.h>

#include "Questions.h"
#include "utilities.h"
#include "json.hpp"
#include "debug.h"

using namespace std;

Questions *Questions::_ps_instance = NULL;

Questions::Questions() {
    if (_ps_instance == NULL)
        _ps_instance = this;
}

Questions *Questions::get_instance() {
    if (_ps_instance == NULL)
        _ps_instance = new Questions();
    
    return _ps_instance;
}

string Questions::get_question(int count) {
    nlohmann::json ret;
    
    string buffer = read_file(QUESTIONS_SOURCE_FILE);
    
    mlog.log_debug("buffer file: %s", buffer.c_str());
    
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