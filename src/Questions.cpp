#include <stdio.h>

#include "Questions.h"
#include "GameDAL.h"
#include "utilities.h"
#include "json.hpp"
#include "debug.h"

#define QUESTION_COUNT (10)

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
    
    for (int i = 0; i < count; i++) {
        // pick random question
        int idx = rand()%QUESTION_COUNT + 1;
        
        QuestionInfo qi = GameDAL::get_instance()->get_question_by_id(idx);
        
        nlohmann::json qi_json;
        qi_json["question"] = qi.question;
        qi_json["a"] = qi.choice_a;
        qi_json["b"] = qi.choice_b;
        qi_json["c"] = qi.choice_c;
        qi_json["d"] = qi.choice_d;
        qi_json["answer"] = qi.correct_answer;
        
        // add to return json object
        ret["questions"].push_back(qi_json);
    }
    
    return ret.dump();
}