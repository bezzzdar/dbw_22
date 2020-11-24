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

bool Connector::CheckUserAnswer(const int user_id, const Disciplines& discipline,
                                const std::string& answer_user) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    int number_discipline = RequestUserNumberDiscipline(user_id, discipline);

    sql_request << "SELECT " << discipline_name << "_answer FROM dialogue2020." << discipline_name
                << " WHERE " << discipline_name << "_id=" << number_discipline;

    sql::ResultSet* res_answer = stmt_->executeQuery(sql_request.str().c_str());

    std::string answer{};
    while (res_answer->next()) {
        answer = res_answer->getString(1);
    }
    delete (res_answer);

    if (answer.find(answer_user) != std::string::npos) {
        return true;
    }

    return false;
}

std::string Connector::RequestUserTask(const int user_id, const Disciplines& discipline,
                                       const int type /* = 0*/) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    int number_discipline = RequestUserNumberDiscipline(user_id, discipline);

    sql_request << "SELECT " << discipline_name << "_task FROM dialogue2020." << discipline_name
                << " WHERE " << discipline_name << "_id=" << number_discipline;

    sql::ResultSet* res_task = stmt_->executeQuery(sql_request.str().c_str());

    std::string task{};
    while (res_task->next()) {
        task = res_task->getString(1);
    }
    delete (res_task);

    return task;
}

int Connector::RequestUserNumberDiscipline(const int user_id, const Disciplines& discipline) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    sql_request << "SELECT number_" << discipline_name
                << " FROM dialogue2020.users WHERE user_id=" << user_id;

    sql::ResultSet* res_number_discipline = stmt_->executeQuery(sql_request.str().c_str());

    int number_discipline = 1;
    while (res_number_discipline->next()) {
        number_discipline = res_number_discipline->getInt(1);
    }

    delete (res_number_discipline);

    return number_discipline;
}

void Connector::RegisterCorrectAnswer(const int user_id, const Disciplines& discipline,
                                      /* out */ bool* no_more /* = nullptr */) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    sql_request << "UPDATE dialogue2020.users SET number_" << discipline_name << "=number_"
                << discipline_name << "+1 WHERE user_id=" << user_id;
    stmt_->execute(sql_request.str().c_str());
    sql_request.str("");

    sql_request << "UPDATE dialogue2020.users SET score"
                << "=score+1 WHERE user_id=" << user_id;
    stmt_->execute(sql_request.str().c_str());
    sql_request.str("");

    if (no_more != nullptr) {
        sql_request << "SELECT COUNT(" << discipline_name << "_id) FROM dialogue2020."
                    << discipline_name;

        sql::ResultSet* res_n_questions = stmt_->executeQuery(sql_request.str().c_str());

        int n_questions = 0;
        while (res_n_questions->next()) {
            n_questions = res_n_questions->getInt(1);
        }

        delete (res_n_questions);
        sql_request.str("");

        int number_discipline = RequestUserNumberDiscipline(user_id, discipline);

        if (number_discipline > n_questions) {
            *no_more = true;
        } else {
            *no_more = false;
        }
    }

    return;
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