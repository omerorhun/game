#include <stdio.h>
#include <mutex>

#include "RegistryDAO.h"

#include "debug.h"

#define TABLE_NAME "Users"
#define COL_ID "id"
#define COL_FB_ID "fb_id"
#define COL_NAME "name"
#define COL_PICTURE "picture"

using namespace std;

mutex mtx_db;

RegistryDAO *RegistryDAO::_ps_instance = NULL;

RegistryDAO::RegistryDAO(sql::Connection *conn) {
    if (_ps_instance == NULL) {
        _ps_instance = this;
        _conn = conn;
    }
}

RegistryDAO *RegistryDAO::get_instance(sql::Connection *conn) {
    if (_ps_instance == NULL) {
        _ps_instance = new RegistryDAO(conn);
    }
    
    return _ps_instance;
}

void RegistryDAO::insert(RegistryInfo user_info) {
    mtx_db.lock();
    sql::SQLString query = "INSERT INTO Users (fb_id, name, picture) VALUES (?,?,?)";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    
    pstmt->setUInt64(1, user_info.fb_id);
    pstmt->setString(2, user_info.name);
    pstmt->setString(3, user_info.picture_url);
    
    pstmt->execute();
    
    delete pstmt;
    mtx_db.unlock();
}

RegistryInfo RegistryDAO::get_user_by_uid(uint64_t uid) {
    mtx_db.lock();
    sql::SQLString query = "SELECT * FROM Users WHERE id = (?)";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    pstmt->setUInt64(1, uid);
    sql::ResultSet *res = pstmt->executeQuery();
    RegistryInfo reg_info;
    
    if (res->next()) {
        reg_info.uid = res->getUInt64(COL_ID);
        reg_info.fb_id = res->getUInt64(COL_FB_ID);
        reg_info.name = res->getString(COL_NAME);
        reg_info.picture_url = res->getString(COL_PICTURE);
    }
    
    delete res;
    delete pstmt;
    mtx_db.unlock();
    return reg_info;
}

bool RegistryDAO::check_registry_by_uid(uint64_t uid) {
    mtx_db.lock();
    bool ret = false;
    sql::SQLString query = "SELECT EXISTS(SELECT * FROM Users WHERE id = (?)";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    pstmt->setUInt64(1, uid);
    sql::ResultSet *res = pstmt->executeQuery();
    
    if (res->next()) {
        if (res->getRow() && (res->getInt(1) == 1)) {
            ret = true;
        }
    }
    
    delete res;
    delete pstmt;
    mtx_db.unlock();
    return ret;
}

RegistryInfo RegistryDAO::get_user_by_fb_id(uint64_t fb_id) {
    mtx_db.lock();
    RegistryInfo ret;
    sql::SQLString query = "SELECT * FROM Users WHERE fb_id = (?)";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    pstmt->setUInt64(1, fb_id);
    sql::ResultSet *res = pstmt->executeQuery();
    
    if (res->next()) {
        ret.uid = res->getUInt64(COL_ID);
        ret.fb_id = res->getUInt64(COL_FB_ID);
        ret.name = res->getString(COL_NAME);
        ret.picture_url = res->getString(COL_PICTURE);
    }
    
    delete res;
    delete pstmt;
    mtx_db.unlock();
    return ret;
}

bool RegistryDAO::check_registry_by_fb_id(uint64_t fb_id) {
    mtx_db.lock();
    bool ret = false;
    sql::SQLString query = "SELECT EXISTS(SELECT * FROM Users WHERE fb_id = (?))";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    pstmt->setUInt64(1, fb_id);
    sql::ResultSet *res = pstmt->executeQuery();
    
    if (res->next()) {
        if (res->getRow() && (res->getInt(1) == 1)) {
            ret = true;
        }
    }
    
    delete res;
    delete pstmt;
    mtx_db.unlock();
    return ret;
}

void RegistryDAO::removeUser(uint64_t uid) {
    
}
