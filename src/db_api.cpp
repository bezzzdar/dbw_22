#include "db_api.h"

#include <sstream>

namespace db_api {
int Connector::AddUser(std::string name, int school_n) {
    std::stringstream sql_request;

    sql_request << "SELECT COUNT(*) FROM dialogue2020.users";
    sql::ResultSet* res = stmt_->executeQuery(sql_request.str().c_str());

    int user_id = -1;
    while (res->next()) {
        user_id = res->getInt(1);
    }

    sql_request.str("");
    delete (res);

    sql_request << "INSERT INTO dialogue2020.users(user_id, user_name, school) VALUES (" << user_id
                << ", \'" << name.c_str() << "\', " << school_n << ")";
    stmt_->execute(sql_request.str().c_str());

    return user_id;
}

int Connector::RemoveUser(int user_id) {
    std::stringstream sql_request;

    sql_request << "DELETE FROM dialogue2020.users WHERE user_id=" << user_id;
    stmt_->execute(sql_request.str().c_str());
}

int Connector::CheckUserAnswer(int user_id, std::string answer) {
}

int Connector::RequestUserTask(int user_id, Disciplines discipline) {
}

int Connector::RequestUserScore(int user_id) {
}
}; // namespace db_api