#include "db_api.h"
#include "bot_utils.h"

#include <iostream>
#include <sstream>
#include <string>


namespace db_api {
int AllTryCounter = 0;
int RightAnswerCounter = 0;


int Connector::AddUser(const std::string& name, const int school_n, const int grade_n) {
    std::stringstream sql_request;

    sql_request << "SELECT COUNT(*) FROM dialogue2022.users;";
    sql::ResultSet* res = stmt_->executeQuery(sql_request.str().c_str());

    int user_id = -1;
    while (res->next()) {
        user_id = res->getInt(1) + 1;
    }

    sql_request.str("");
    delete (res);

    sql_request << "INSERT INTO dialogue2022.users(user_id, user_name, school, grade) VALUES (" << user_id
                << ", '" << name.c_str() << "', " << school_n << ", " << grade_n << ");";
                
    stmt_->execute(sql_request.str().c_str());

    return user_id;
}

int Connector::relativeIdToAbsoluteId(const int task, const int grade, const std::string& name)
{
    std::stringstream sql_request;
    int absoluteId;

    sql_request << "SELECT id FROM dialogue2022." << name.c_str() << " WHERE grade_number=" << grade << " LIMIT 1;";
    std::cout << "sql_request from relativeIdToAbsoluteId = " << sql_request.str() << "\n";
    
    //запрашиваем id первой строчки, где grade = grade 
    sql::ResultSet* res_absoluteId = stmt_->executeQuery(sql_request.str().c_str());
    while (res_absoluteId->next()) {
        absoluteId = res_absoluteId->getInt(1);        
    }
    delete(res_absoluteId);
    sql_request.str("");

    absoluteId = absoluteId+task-1;

    return absoluteId;
}

bool Connector::UsernameTaken(const std::string& name) {
    std::stringstream sql_request;

    sql_request << "SELECT COUNT(1) FROM dialogue2022.users WHERE user_name=\""
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

    sql_request << "DELETE FROM dialogue2022.users WHERE user_id=" << user_id;
    stmt_->execute(sql_request.str().c_str());

    return;
}

bool Connector::CheckAnswer(const std::string& user_answer, const Disciplines& discipline,
                            const size_t n_task, const int n_grade) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));
    
    int               absoluteId = -1; 
    int               n_grade_db = -1;
    
    //increase counter of all answers
    AllTryCounter++;

    std::cout << "Check answer \n";    

    switch (discipline)
    {
        case CULT: case GEO: case SOCIAL:
            n_grade_db = 0;
            sql_request << "SELECT "
                    << "is_case_sensitive FROM dialogue2022." << discipline_name << " WHERE "
                    << "id=" << n_task << ";";
            std::cout << "sql_request = " << sql_request.str() << "\n";    
            break;
        default:
            if(n_grade <= 9) {
                n_grade_db=9;
                absoluteId = relativeIdToAbsoluteId(n_task, n_grade_db, discipline_name);        
            }
            else {
                n_grade_db = 10;
                absoluteId = relativeIdToAbsoluteId(n_task, n_grade_db, discipline_name);
            }
            sql_request << "SELECT is_case_sensitive FROM dialogue2022." << discipline_name << " WHERE "
                << "id=" << absoluteId <<";";
            std::cout << "sql_request = " << sql_request.str() << "\n";    
            break;
    }            
    sql::ResultSet* res_case = stmt_->executeQuery(sql_request.str().c_str());

    bool is_case_sensitive = false;
    while (res_case->next()) {
        is_case_sensitive = res_case->getInt(1);
    }
    
    sql_request.str("");
    delete (res_case);

    const std::string ans = (is_case_sensitive) ? (bot_utils::NoSpaces(user_answer))
                                                : (bot_utils::ToLowerNoSpaces(user_answer));

                                                

    std::cout << "user answer (is_case_sensitive: " << is_case_sensitive << "): <" << ans << ">\n";


    switch (discipline)
    {
        case CULT: case GEO: case SOCIAL:
            n_grade_db = 0;
            sql_request << "SELECT "
                << "answer FROM dialogue2022." << discipline_name << " WHERE "
                << "id=" << n_task << ";";
            std::cout << "sql_request = " << sql_request.str() << "\n";    
            break;
        default:
            if(n_grade <= 9) {
                n_grade_db=9;
                absoluteId = relativeIdToAbsoluteId(n_task, n_grade_db, discipline_name);        
            }
            else {
                n_grade_db = 10;
                absoluteId = relativeIdToAbsoluteId(n_task, n_grade_db, discipline_name);
            }
            sql_request << "SELECT "
                << "answer FROM dialogue2022." << discipline_name << " WHERE "
                << "id=" << absoluteId;
                std::cout << "sql_request = " << sql_request.str() << "\n";    
            break;
    }    

    sql::ResultSet* res_answer = stmt_->executeQuery(sql_request.str().c_str());

    std::string answer{};
    while (res_answer->next()) {
        answer = res_answer->getString(1);
    }
    delete (res_answer);

    std::vector<std::string> answers = bot_utils::Parse(answer, '@');

    std::cout << "parsed answers for " << discipline_name << absoluteId << ": ";
    for (const auto& token : answers) {
        std::cout << '<' << token << ">\n";
    }

    for (const auto& token : answers) {
        if (token == ans) {
            std::cout << "correct\n";
            
            //increase counter of correct answers
            RightAnswerCounter++;

            return true;
        }
    }

    std::cout << "incorrect\n";
    return false;
}

void Connector::NumberAnswers(int& allAnswers, int& correctAnswers)
{
    allAnswers=AllTryCounter;
    correctAnswers=RightAnswerCounter;

    return;
}

Task Connector::RequestTask(const Disciplines& discipline, const size_t n_task, const int n_grade) {
   
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));
    int               absoluteId = -1; //= relativeIdToAbsoluteId(n_task, n_grade, discipline_name);
    int               n_grade_db = 0;
   
    switch (discipline)
    {
        case CULT: case GEO: case SOCIAL:
            n_grade_db = 0;
            sql_request << "SELECT "
                    << "task, picture FROM dialogue2022." << discipline_name << " WHERE "
                    << "id=" << n_task << " AND " << "grade_number=" << n_grade_db << ";";
            std::cout << "sql_request = " << sql_request.str() << "\n";    
            break;                   
        default:
            if(n_grade <= 9) {
                n_grade_db=9;
                absoluteId = relativeIdToAbsoluteId(n_task, n_grade_db, discipline_name);        
            }
            else {
                n_grade_db = 10;
                absoluteId = relativeIdToAbsoluteId(n_task, n_grade_db, discipline_name);
            }
            sql_request << "SELECT "
                    << "task, picture FROM dialogue2022." << discipline_name << " WHERE "
                    << "id=" << absoluteId << " AND " << "grade_number=" << n_grade_db << ";";
            std::cout << "sql_request = " << sql_request.str() << "\n";
            break;
    }
    sql::ResultSet* res_task = stmt_->executeQuery(sql_request.str().c_str());

    Task task{};
    while (res_task->next()) {
        task.text = res_task->getString(1);
        task.pic_name = res_task->getString(2);
    }
    delete (res_task);

    return task;
}

void Connector::RegisterCorrectAnswer(const int user_id, const Disciplines& discipline,
                                      const size_t task_id, const int n_grade) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));

    int               absoluteId = -1; //= relativeIdToAbsoluteId(n_task, n_grade, discipline_name);
    int               n_grade_db = -1;
    
    sql_request << "UPDATE dialogue2022.users SET score"
                << "=score+1 WHERE user_id=" << user_id;
    stmt_->execute(sql_request.str().c_str());
    sql_request.str("");

    switch (discipline)
    {
        case CULT: case GEO: case SOCIAL:
            n_grade_db = 0;
            sql_request << "UPDATE dialogue2022." << discipline_name << " SET solved"
                << "=solved+1 WHERE id=" << task_id;
            std::cout << "sql_request = " << sql_request.str() << "\n";    
            break;
        default:
            if(n_grade<=9) {
                n_grade_db=9;
                absoluteId = relativeIdToAbsoluteId(task_id, n_grade_db, discipline_name);        
            }
            else {
                n_grade_db = 10;
                absoluteId = relativeIdToAbsoluteId(task_id, n_grade_db, discipline_name);
            }  
            sql_request << "UPDATE dialogue2022." << discipline_name << " SET solved"
                        << "=solved+1 WHERE id=" << absoluteId;
            std::cout << "sql_request = " << sql_request.str() << "\n";    
            break;
    }
    stmt_->execute(sql_request.str().c_str());
    sql_request.str("");

    return;
}

int Connector::RequestNumberTasks(const Disciplines& discipline, int grade) {
    std::stringstream sql_request;
    std::string       discipline_name(discipline_to_string.at(discipline));
    int               n_grade_db=0;  
    if(grade == 0)    {
        n_grade_db = 0; 
    } else if (grade >= 6 && grade <= 9) {
        n_grade_db = 9;                
    } else {
        n_grade_db = 10;                
    } 
    sql_request << "SELECT COUNT("
                << "id) FROM dialogue2022." << discipline_name << " WHERE grade_number=" << n_grade_db;

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

    sql_request << "SELECT score FROM dialogue2022.users WHERE user_id=" << user_id;
    std::cout << " sql_request for RequestUserScore " << sql_request.str() << "\n";

    sql::ResultSet* res_score = stmt_->executeQuery(sql_request.str().c_str());

    int score = 0;
    while (res_score->next()) {
        score = res_score->getInt(1);
    }
    delete (res_score);

    return score;
}

}; // namespace db_api