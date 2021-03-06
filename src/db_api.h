#ifndef __DIALOGUE_DB_API_H_INCLUDED__
#define __DIALOGUE_DB_API_H_INCLUDED__

// #include <cppconn/driver.h>
// #include <cppconn/prepared_statement.h>

#include <sstream>
#include <iostream>

#include <mysql/jdbc.h>
#include <mysqlx/xdevapi.h>

#include <map>
#include <string>

namespace db_api {
struct Task {
    std::string text;
    std::string pic_name;
};

enum Disciplines {
    PHY,
    MATH,
    GEO,
    SOCIAL,
    BIO,
    COD,
    CULT,
    HIST,
    CHEM,
    ENG,
    NONE,
};

const std::map<Disciplines, const char*> discipline_to_string{
    {PHY, "phy"},
    {MATH, "math"},    
    {BIO, "bio"},
    {COD, "cod"},
    {CULT, "cult"},
    {HIST, "hist"},
    {CHEM, "chem"},
    {ENG, "eng"},
    {GEO, "geo"},
    {SOCIAL, "social"},
    {NONE, "none"},
};

class Connector {
  public:
    Connector() = delete;
    Connector(const char* hostname, const char* username, const char* password,
              const char* db_name) {
        try {
            driver_ = get_driver_instance();

            con_ = driver_->connect(hostname, username, password);

            con_->setSchema(db_name);

            stmt_ = con_->createStatement();
        } catch (sql::SQLException& e) {
            std::cerr << "# ERR: SQLException in " << __FILE__;
            std::cerr << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
            std::cerr << "# ERR: " << e.what();
            std::cerr << " (MySQL error code: " << e.getErrorCode();
            std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        } catch (std::exception& ex) {
            std::cerr << "STD EXCEPTION: " << ex.what() << std::endl;
        } catch (const char* ex) {
            std::cerr << "EXCEPTION: " << ex << std::endl;
        }
    }

    ~Connector() {
        delete stmt_;
        delete con_;
    }
    int relativeIdToAbsoluteId(const int task, const int grade, const std::string& name);
    
    int  AddUser(const std::string& name, const int school_n, const int grade_n, const int category);
    bool UsernameTaken(const std::string& name);
    void RemoveUser(const int user_id);
    bool CheckAnswer(const std::string& user_answer, const Disciplines& discipline,
                     const size_t n_task, const int n_grade);
    void NumberAnswers(int& allAnswers, int& correctAnswers, int category);
    Task RequestTask(const Disciplines& discipline, const size_t n_task, const int n_grade);
    void RegisterCorrectAnswer(const int user_id, const Disciplines& discipline, const size_t task_id, const int n_grade);
    int  RequestNumberTasks(const Disciplines& discipline, int grade);

    int RequestUserScore(const int user_id);
    //std::vector<std::pair<std::string,int>> Connector::GetUsersWithScore();
    

  private:
    sql::Driver*     driver_;
    sql::Connection* con_;
    sql::Statement*  stmt_;
};
}; // namespace db_api

#endif