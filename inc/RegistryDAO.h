#ifndef _REGISTRY_DAO_H_
#define _REGISTRY_DAO_H_

#include <mysql-cppconn-8/jdbc/cppconn/driver.h>
#include <mysql-cppconn-8/jdbc/cppconn/connection.h>
#include <mysql-cppconn-8/jdbc/cppconn/statement.h>
#include <mysql-cppconn-8/jdbc/cppconn/prepared_statement.h>
#include <mysql-cppconn-8/jdbc/cppconn/resultset.h>

#include <string>

typedef struct {
    uint64_t uid;
    uint64_t fb_id;
    std::string name;
    std::string picture_url;
}RegistryInfo;

class RegistryDAO {
    
    public:
    RegistryDAO(sql::Connection *conn);
    ~RegistryDAO();
    
    static RegistryDAO *get_instance(sql::Connection *conn);
    
    void insert(RegistryInfo user_info);
    RegistryInfo get_user_by_fb_id(uint64_t fb_id);
    bool check_registry_by_fb_id(uint64_t fb_id);
    RegistryInfo get_user_by_uid(uint64_t uid);
    bool check_registry_by_uid(uint64_t uid);
    
    void updateUser(uint64_t uid, RegistryInfo user_info);
    void removeUser(uint64_t uid);
    
    private:
    static RegistryDAO *_ps_instance;
    sql::Connection *_conn;
    
};

#endif // _REGISTRY_DAO_H_