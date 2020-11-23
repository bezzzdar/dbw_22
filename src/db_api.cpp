#include "db_api.h"
#include "bot_utils.h"

#include <sstream>

namespace db_api {
int Connector::AddUser(std::string name, int school_n) {
    std::stringstream sql_request;

    sql_request << "SELECT COUNT(*) FROM dialogue2020.users";
    sql::ResultSet* res = stmt_->executeQuery(sql_request.str().c_str());

    int user_id = -1;
    while (res->next()) {
        user_id = res->getInt(1) + 1;
    }

    sql_request.str("");
    delete (res);

    sql_request << "INSERT INTO dialogue2020.users(user_id, user_name, school) VALUES (" << user_id
                << ", \'" << name.c_str() << "\', " << school_n << ")";
    stmt_->execute(sql_request.str().c_str());

    return user_id;
}

bool Connector::UsernameTaken(std::string name) {
    std::stringstream sql_request;

    name = bot_utils::ToLowerNoSpaces(name);

    sql_request << "SELECT COUNT(1) FROM dialogue2020.users WHERE user_name=\"" << name << "\"";

    sql::ResultSet* res = stmt_->executeQuery(sql_request.str().c_str());

    int count = 0;
    while (res->next()) {
        count = res->getInt(1);
    }

    delete (res);

    return count == 1;
}

int Connector::RemoveUser(int user_id) {
    std::stringstream sql_request;

    sql_request << "DELETE FROM dialogue2020.users WHERE user_id=" << user_id;
    stmt_->execute(sql_request.str().c_str());
}

int Connector::CheckUserAnswer(int user_id, std::string answer) {
}

int Connector::RequestUserTask(int user_id, Disciplines discipline, int type /* = 0*/) {
    std::stringstream sql_request;

    std::string discipline_name(discipline_to_string.at(discipline));

    sql_request << "SELECT number_" << discipline_name
                << " FROM dialogue2020.users WHERE user_id=" << user_id;

    sql::ResultSet* res_number_discipline = stmt_->executeQuery(sql_request.str().c_str());

    int number_discipline = 0;
    while (res_number_discipline->next()) {
        number_discipline = res_number_discipline->getInt(1);
    }

    sql_request.str("");
    delete (res_number_discipline);

    sql_request << "SELECT " << discipline_name << "_task FROM dialogue2020." << discipline_name
                << " WHERE " << discipline_name << "_id=" << number_discipline;

    sql::ResultSet* res_task = stmt_->executeQuery(sql_request.str().c_str());

    std::string task{};
    while (res_task->next()) {
        task = res_task->getString(1);
    }
    delete (res_task);

    std::cout << "<" << task << ">\n";

    return 1;
}

int Connector::RequestUserScore(int user_id) {
}

}; // namespace db_api