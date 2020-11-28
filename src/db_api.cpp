#include "db_api.h"
#include "bot_utils.h"

#include <sstream>

namespace db_api {
int Connector::AddUser(const std::string& name, const int school_n) {
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

bool Connector::UsernameTaken(const std::string& name) {
    std::stringstream sql_request;

    sql_request << "SELECT COUNT(1) FROM dialogue2020.users WHERE user_name=\""
                << bot_utils::ToLowerNoSpaces(name) << "\"";

    sql::ResultSet* res = stmt_->executeQuery(sql_request.str().c_str());

    int count = 0;
    while (res->next()) {
        count = res->getInt(1);
    }

    delete (res);

    return count == 1;
}

void Connector::RemoveUser(const int user_id) {
    std::stringstream sql_request;

    sql_request << "DELETE FROM dialogue2020.users WHERE user_id=" << user_id;
    stmt_->execute(sql_request.str().c_str());

    return;
}

bool Connector::CheckAnswer(const std::string& user_answer, const Disciplines& discipline,
                            const size_t n_task) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    sql_request << "SELECT " << discipline_name << "_answer FROM dialogue2020." << discipline_name
                << " WHERE " << discipline_name << "_id=" << n_task;

    sql::ResultSet* res_answer = stmt_->executeQuery(sql_request.str().c_str());

    std::string answer{};
    while (res_answer->next()) {
        answer = res_answer->getString(1);
    }
    delete (res_answer);

    std::vector<std::string> answers = bot_utils::Parse(answer, '@');

    for (const auto& token : answers) {
        std::cout << discipline_name << n_task << '<' << token << ">\n";
    }

    for (const auto& token : answers) {
        if (token == user_answer) {
            return true;
        }
    }

    return false;
}

std::string Connector::RequestTask(const Disciplines& discipline, const size_t n_task) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    sql_request << "SELECT " << discipline_name << "_task FROM dialogue2020." << discipline_name
                << " WHERE " << discipline_name << "_id=" << n_task;

    sql::ResultSet* res_task = stmt_->executeQuery(sql_request.str().c_str());

    std::string task{};
    while (res_task->next()) {
        task = res_task->getString(1);
    }
    delete (res_task);

    return task;
}

void Connector::RegisterCorrectAnswer(const int user_id, const Disciplines& discipline) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    sql_request << "UPDATE dialogue2020.users SET score"
                << "=score+1 WHERE user_id=" << user_id;
    stmt_->execute(sql_request.str().c_str());
    sql_request.str("");

    return;
}

int Connector::RequestNumberTasks(const Disciplines& discipline) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    sql_request << "SELECT COUNT(" << discipline_name << "_id) FROM dialogue2020."
                << discipline_name;

    sql::ResultSet* res_n_questions = stmt_->executeQuery(sql_request.str().c_str());

    int n_questions = 0;
    while (res_n_questions->next()) {
        n_questions = res_n_questions->getInt(1);
    }

    delete (res_n_questions);
    sql_request.str("");

    return n_questions;
}

int Connector::RequestUserScore(const int user_id) {
    std::stringstream sql_request;

    sql_request << "SELECT score FROM dialogue2020.users WHERE user_id=" << user_id;

    sql::ResultSet* res_score = stmt_->executeQuery(sql_request.str().c_str());

    int score = 0;
    while (res_score->next()) {
        score = res_score->getInt(1);
    }
    delete (res_score);

    return score;
}

}; // namespace db_api