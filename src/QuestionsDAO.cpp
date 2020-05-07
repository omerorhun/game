#include <stdio.h>
#include <mutex>

#include "QuestionsDAO.h"

#include "debug.h"

#define TABLE_NAME "Questions"
#define COL_ID "id"
#define COL_QUESTION_TR "question_tr"
#define COL_CHOICE_A "choice_a"
#define COL_CHOICE_B "choice_b"
#define COL_CHOICE_C "choice_c"
#define COL_CHOICE_D "choice_d"
#define COL_CORRECT_ANSWER "correct_answer"

using namespace std;

extern mutex g_sql_mtx;

QuestionsDAO *QuestionsDAO::_ps_instance = NULL;

QuestionsDAO::QuestionsDAO(sql::Connection *conn) {
    if (_ps_instance == NULL) {
        _ps_instance = this;
        _conn = conn;
    }
}

QuestionsDAO *QuestionsDAO::get_instance(sql::Connection *conn) {
    if (_ps_instance == NULL) {
        _ps_instance = new QuestionsDAO(conn);
    }
    
    return _ps_instance;
}

QuestionInfo QuestionsDAO::get_question_by_id(uint64_t id) {
    g_sql_mtx.lock();
    sql::SQLString query = "SELECT * FROM Questions WHERE id = (?)";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    pstmt->setUInt64(1, id);
    sql::ResultSet *res = pstmt->executeQuery();
    QuestionInfo question_info;
    // "SELECT * FROM Questions WHERE category = 1 ORDER BY RAND() LIMIT 1"
    if (res->next()) {
        question_info.uid = res->getUInt64(COL_ID);
        question_info.question = res->getString(COL_QUESTION_TR);
        question_info.choice_a = res->getString(COL_CHOICE_A);
        question_info.choice_b = res->getString(COL_CHOICE_B);
        question_info.choice_c = res->getString(COL_CHOICE_C);
        question_info.choice_d = res->getString(COL_CHOICE_D);
        question_info.correct_answer = res->getString(COL_CORRECT_ANSWER);
    }
    
    delete res;
    delete pstmt;
    g_sql_mtx.unlock();
    return question_info;
}

QuestionInfo QuestionsDAO::get_random_question(uint8_t category) {
    g_sql_mtx.lock();
    sql::SQLString query = "SELECT * FROM Questions WHERE category = (?) ORDER BY RAND() LIMIT 1";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    pstmt->setUInt(1, category);
    sql::ResultSet *res = pstmt->executeQuery();
    QuestionInfo question_info;
    
    if (res->next()) {
        question_info.uid = res->getUInt64(COL_ID);
        question_info.question = res->getString(COL_QUESTION_TR);
        question_info.choice_a = res->getString(COL_CHOICE_A);
        question_info.choice_b = res->getString(COL_CHOICE_B);
        question_info.choice_c = res->getString(COL_CHOICE_C);
        question_info.choice_d = res->getString(COL_CHOICE_D);
        question_info.correct_answer = res->getString(COL_CORRECT_ANSWER);
    }
    
    delete res;
    delete pstmt;
    g_sql_mtx.unlock();
    
    return question_info;
}