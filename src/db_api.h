#ifndef __DIALOGUE_DB_API_H_INCLUDED__
#define __DIALOGUE_DB_API_H_INCLUDED__

// #include <cppconn/driver.h>
// #include <sstream>
// #include <cppconn/prepared_statement.h>

#include <iostream>
#include <mysql-cppconn-8/mysql/jdbc.h>
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
    RUS,
    BIO,
    COD,
    GEN,
    HIST,
    CHEM,
    SOC,
    NONE,
};

const std::map<Disciplines, const char*> discipline_to_string{
    {PHY, "phy"},
    {MATH, "math"},
    {RUS, "rus"},
    {BIO, "bio"},
    {COD, "cod"},
    {GEN, "gen"},
    {HIST, "hist"},
    {CHEM, "chem"},
    {SOC, "soc"},
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

    int  AddUser(const std::string& name, const int school_n);
    bool UsernameTaken(const std::string& name);
    void RemoveUser(const int user_id);
    bool CheckAnswer(const std::string& user_answer, const Disciplines& discipline,
                     const size_t n_task);
    Task RequestTask(const Disciplines& discipline, const size_t n_task);
    void        RegisterCorrectAnswer(const int user_id, const Disciplines& discipline);
    int         RequestNumberTasks(const Disciplines& discipline);

    int RequestUserScore(const int user_id);

  private:
    sql::Driver*     driver_;
    sql::Connection* con_;
    sql::Statement*  stmt_;
};
}; // namespace db_api

#endif