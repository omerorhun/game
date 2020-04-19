#include "GameDAL.h"

#define DATABASE_HOSTNAME "localhost"
#define DATABASE_NAME "testdb"
#define DATABASE_USERNAME "root"
#define DATABASE_PASSWORD "kar12345"

using namespace std;

GameDAL *GameDAL::_ps_instance = NULL;

GameDAL::GameDAL() {
    if (_ps_instance == NULL) {
        _driver = get_driver_instance();
        _conn = _driver->connect(DATABASE_HOSTNAME, DATABASE_USERNAME, DATABASE_PASSWORD);
        _p_reg_dao = RegistryDAO::get_instance(_conn);
        _conn->setSchema(DATABASE_NAME);
    }
}

GameDAL *GameDAL::get_instance() {
    if (_ps_instance == NULL) {
        _ps_instance = new GameDAL();
    }
    
    return _ps_instance;
}

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


