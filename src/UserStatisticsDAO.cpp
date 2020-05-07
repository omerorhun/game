#include "UserStatisticsDAO.h"
#include <sstream>

#define USER_STATISTICS_COL_USER_ID "user_id"
#define USER_STATISTICS_COL_RIGHT "correct"
#define USER_STATISTICS_COL_WRONG "wrong"
#define USER_STATISTICS_COL_MAX_ROW "max_row"
#define USER_STATISTICS_COL_ROW "row"

using namespace std;

UserStatisticsDAO *UserStatisticsDAO::_ps_instance = NULL;

UserStatisticsDAO::UserStatisticsDAO(sql::Connection *conn) {
    if (_ps_instance == NULL) {
        _ps_instance = this;
        _conn = conn;
    }
}

UserStatisticsDAO *UserStatisticsDAO::get_instance(sql::Connection *conn) {
    if (_ps_instance == NULL) {
        _ps_instance = new UserStatisticsDAO(conn);
    }
    
    return _ps_instance;
}

void UserStatisticsDAO::insert_user_stat(uint64_t uid) {
    insert_user_with_category(uid, 0); // for main table
    insert_user_with_category(uid, 1); // for test category
}

void UserStatisticsDAO::insert_user_with_category(uint64_t uid, uint8_t cat) {
    sql::SQLString query = "INSERT INTO " + get_table_name(cat) + " (user_id) VALUES(?);";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    
    pstmt->setUInt64(1, uid);
    pstmt->executeQuery();
    
    delete pstmt;
    
    return;
}

void UserStatisticsDAO::update_user_stat(uint64_t uid, uint8_t cat, UserStatisticsInfo usi) {
    sql::SQLString query = "UPDATE " + get_table_name(cat) + 
                            " SET correct=correct+(?), wrong=wrong+(?) WHERE user_id = (?);";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    
    pstmt->setInt(1, usi.right);
    pstmt->setInt(2, usi.wrong);
    pstmt->setUInt64(3, uid);
    pstmt->executeQuery();
    
    delete pstmt;
    
    return;
}

void UserStatisticsDAO::update_user_win_lose(uint64_t uid, uint8_t cat, WinLoseEven wle) {
    sql::SQLString query = "UPDATE " + get_table_name(cat) + " SET " 
                                + get_wle_state(wle) + " WHERE user_id = (?);";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    
    pstmt->setUInt64(1, uid);
    pstmt->executeQuery();
    
    delete pstmt;
    
    return;
}

UserStatisticsInfo UserStatisticsDAO::get_user_stat(uint64_t uid, uint8_t cat) {
    sql::SQLString query = "SELECT * FROM " + get_table_name(cat) + " WHERE user_id = (?);";
    sql::PreparedStatement *pstmt = _conn->prepareStatement(query);
    pstmt->setUInt64(1, uid);
    sql::ResultSet *res = pstmt->executeQuery();
    
    UserStatisticsInfo usi;
    
    if (res->next()) {
        usi.right = res->getInt(USER_STATISTICS_COL_RIGHT);
        usi.wrong = res->getInt(USER_STATISTICS_COL_WRONG);
    }
    
    delete res;
    delete pstmt;
    
    return usi;
}

string UserStatisticsDAO::get_table_name(uint8_t category) {
    string table_name = "";
    switch (category) {
        case 0:
            table_name = "UserStatistics";
        default:
            table_name = "TestCategoryStat";
            break;
    }
    
    return table_name;
}

string UserStatisticsDAO::get_wle_state(WinLoseEven wle) {
    string ret_wle = "";
    switch (wle) {
        case GAME_LOSE:
            ret_wle = "lose=lose+1";
            break;
        case GAME_WIN:
            ret_wle = "win=win+1";
            break;
        case GAME_EVEN:
            ret_wle = "even=even+1";
            break;
    }
    
    return ret_wle;
}