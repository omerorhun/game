#ifndef _USER_STATISTICS_DAO_H_
#define _USER_STATISTICS_DAO_H_

#include <mysql-cppconn-8/jdbc/cppconn/driver.h>
#include <mysql-cppconn-8/jdbc/cppconn/connection.h>
#include <mysql-cppconn-8/jdbc/cppconn/statement.h>
#include <mysql-cppconn-8/jdbc/cppconn/prepared_statement.h>
#include <mysql-cppconn-8/jdbc/cppconn/resultset.h>

#include <string>

typedef enum {
    GAME_LOSE = 0,
    GAME_WIN = 1,
    GAME_EVEN = 2
}WinLoseEven;

typedef struct {
    uint64_t uid;
    int category;
    int right;
    int wrong;
}UserStatisticsInfo;

class UserStatisticsDAO {
    public:
    UserStatisticsDAO(sql::Connection *conn);
    ~UserStatisticsDAO();
    UserStatisticsInfo get_user_stat(uint64_t uid, uint8_t category);
    void insert_user_stat(uint64_t uid);
    void insert_user_with_category(uint64_t uid, uint8_t category);
    void update_user_stat(uint64_t uid, uint8_t category, UserStatisticsInfo user_stats);
    void update_user_win_lose(uint64_t uid, uint8_t cat, WinLoseEven is_win);
    
    static UserStatisticsDAO *get_instance(sql::Connection *conn);
    
    private:
    static UserStatisticsDAO *_ps_instance;
    sql::Connection *_conn;
    
    std::string get_table_name(uint8_t category);
    std::string get_wle_state(WinLoseEven wle);
};

#endif // _USER_STATISTICS_DAO_H_