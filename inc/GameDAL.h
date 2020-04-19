#ifndef _GAME_DAL_H_
#define _GAME_DAL_H_

#include "RegistryDAO.h"

class GameDAL {
    public:
    GameDAL();
    static GameDAL *get_instance();
    void insert_user(RegistryInfo user_info);
    RegistryInfo get_user_by_fb_id(uint64_t fb_id);
    bool check_registry_by_fb_id(uint64_t fb_id);
    RegistryInfo get_user_by_uid(uint64_t uid);
    bool check_registry_by_uid(uint64_t uid);
    
    private:
    static GameDAL *_ps_instance;
    RegistryDAO *_p_reg_dao;
    sql::Connection *_conn;
    sql::Driver *_driver;
};

#endif // _GAME_DAL_H_