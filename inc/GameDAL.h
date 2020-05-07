#ifndef _GAME_DAL_H_
#define _GAME_DAL_H_

#include "RegistryDAO.h"
#include "QuestionsDAO.h"
#include "UserStatisticsDAO.h"

class GameDAL {
    public:
    GameDAL();
    static GameDAL *get_instance();
    
    // registry
    void insert_user(RegistryInfo user_info);
    RegistryInfo get_user_by_fb_id(uint64_t fb_id);
    bool check_registry_by_fb_id(uint64_t fb_id);
    RegistryInfo get_user_by_uid(uint64_t uid);
    bool check_registry_by_uid(uint64_t uid);
    
    // questions
    QuestionInfo get_question_by_id(uint64_t question_id);
    QuestionInfo get_random_question(uint8_t category);
    
    // user statistics
    UserStatisticsInfo get_user_stat(uint64_t uid, uint8_t category);
    void insert_user_stat(uint64_t uid);
    void update_user_stat(uint64_t uid, uint8_t category, UserStatisticsInfo user_stat);
    void update_user_win_lose(uint64_t uid, uint8_t cat, WinLoseEven wle);
    
    private:
    static GameDAL *_ps_instance;
    RegistryDAO *_p_reg_dao;
    QuestionsDAO *_p_que_dao;
    UserStatisticsDAO *_p_stat_dao;
    
    sql::Connection *_conn;
    sql::Driver *_driver;
};

#endif // _GAME_DAL_H_