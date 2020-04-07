#include <stdio.h>

#include "Questions.h"
#include "utilities.h"
#include "json.hpp"

using namespace std;

Questions::Questions() {
    // open source file
    string buffer = read_file(QUESTIONS_SOURCE_FILE);
    if (nlohmann::json::accept(buffer)) {
        
        nlohmann::json questions = nlohmann::json::parse(buffer);
        
        
    }
}

string Questions::get_question(int count) {
    string ret;
    
    string buffer = read_file(QUESTIONS_SOURCE_FILE);
    nlohmann::json questions = nlohmann::json::parse(buffer);
    
    for (int i = 0; i < count; i++) {
        ret.append(questions.at(i));
    }
    
    
}