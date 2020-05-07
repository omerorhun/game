#include "GameDAL.h"

#define DATABASE_HOSTNAME "localhost"
#define DATABASE_NAME "testdb"
#define DATABASE_USERNAME "orhun"
#define DATABASE_PASSWORD "3536"

using namespace std;

GameDAL *GameDAL::_ps_instance = NULL;

GameDAL::GameDAL() {
    if (_ps_instance == NULL) {
        _driver = get_driver_instance();
        _conn = _driver->connect(DATABASE_HOSTNAME, DATABASE_USERNAME, DATABASE_PASSWORD);
        _p_reg_dao = RegistryDAO::get_instance(_conn);
        _p_que_dao = QuestionsDAO::get_instance(_conn);
        _p_stat_dao = UserStatisticsDAO::get_instance(_conn);
        _conn->setSchema(DATABASE_NAME);
    }
}

GameDAL *GameDAL::get_instance() {
    if (_ps_instance == NULL) {
        _ps_instance = new GameDAL();
    }
    
    return _ps_instance;
}

// registry
void GameDAL::insert_user(RegistryInfo user_info) {
    _p_reg_dao->insert(user_info);
}

RegistryInfo GameDAL::get_user_by_fb_id(uint64_t fb_id) {
    return _p_reg_dao->get_user_by_fb_id(fb_id);
}

bool GameDAL::check_registry_by_fb_id(uint64_t fb_id) {
    return _p_reg_dao->check_registry_by_fb_id(fb_id);
}

RegistryInfo GameDAL::get_user_by_uid(uint64_t uid) {
    return _p_reg_dao->get_user_by_uid(uid);
}

bool GameDAL::check_registry_by_uid(uint64_t uid) {
    return _p_reg_dao->check_registry_by_uid(uid);
}

// questions
QuestionInfo GameDAL::get_question_by_id(uint64_t question_id) {
    return _p_que_dao->get_question_by_id(question_id);
}

QuestionInfo GameDAL::get_random_question(uint8_t category) {
    return _p_que_dao->get_random_question(category);
}

// user statistics
UserStatisticsInfo GameDAL::get_user_stat(uint64_t uid, uint8_t cat) {
    return _p_stat_dao->get_user_stat(uid, cat);
}

void GameDAL::insert_user_stat(uint64_t uid) {
    _p_stat_dao->insert_user_stat(uid);
}

void GameDAL::update_user_stat(uint64_t uid, uint8_t category, UserStatisticsInfo user_stat) {
    _p_stat_dao->update_user_stat(uid, category, user_stat);
}

void GameDAL::update_user_win_lose(uint64_t uid, uint8_t cat, WinLoseEven wle) {
    _p_stat_dao->update_user_win_lose(uid, cat, wle);
}

