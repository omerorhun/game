#ifndef _QUESTIONS_DAO_H_
#define _QUESTIONS_DAO_H_

#include <mysql-cppconn-8/jdbc/cppconn/driver.h>
#include <mysql-cppconn-8/jdbc/cppconn/connection.h>
#include <mysql-cppconn-8/jdbc/cppconn/statement.h>
#include <mysql-cppconn-8/jdbc/cppconn/prepared_statement.h>
#include <mysql-cppconn-8/jdbc/cppconn/resultset.h>

#include <string>

typedef struct {
    uint64_t uid;
    std::string question;
    std::string choice_a;
    std::string choice_b;
    std::string choice_c;
    std::string choice_d;
    std::string correct_answer;
}QuestionInfo;

class QuestionsDAO {
    public:
    QuestionsDAO(sql::Connection *conn);
    ~QuestionsDAO();
    
    static QuestionsDAO *get_instance(sql::Connection *conn);
    
    void insert(QuestionInfo question);
    QuestionInfo get_question_by_id(uint64_t id);
    
    private:
    static QuestionsDAO *_ps_instance;
    sql::Connection *_conn;
};

#endif // _QUESTIONS_DAO_H_