#include <tgbot/tgbot.h>

// int main() {
//     // TgBot::Bot bot("1417068350:AAGHSRRvimiHNWIMgboNm1xUr99D_7-X8gE");

//     // bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
//     //     bot.getApi().sendMessage(message->chat->id, "Hi!");
//     // });

//     // bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
//     //     printf("User wrote %s\n", message->text.c_str());
//     //     if (StringTools::startsWith(message->text, "/start")) {
//     //         return;
//     //     }
//     //     bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
//     // });

//     // try {
//     //     printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
//     //     TgBot::TgLongPoll longPoll(bot);
//     //     while (true) {
//     //         printf("Long poll started\n");
//     //         longPoll.start();
//     //     }
//     // } catch (TgBot::TgException& e) {
//     //     printf("error: %s\n", e.what());
//     // }

//     return 0;
// }

#include <iostream>
#include <mysqlx/xdevapi.h>

#include <mysql-cppconn-8/mysql/jdbc.h>
// #include <cppconn/driver.h>
// #include <cppconn/exception.h>
// #include <cppconn/resultset.h>
// #include <cppconn/statement.h>

constexpr char* USER     = "user@bot.babay.ru";
constexpr char* HOSTNAME = "tcp://bot.babay.ru:3306";
constexpr char* PWD      = "passwd";

constexpr char* USER_LOCAL     = "root";
constexpr char* HOSTNAME_LOCAL = "tcp://LAPTOP-E950M0TH:3306";
constexpr char* PWD_LOCAL      = "vov19411945_qW";

using namespace ::mysqlx;

int main(int argc, const char* argv[]) try {
    sql::Driver*     driver;
    sql::Connection* con;
    sql::Statement*  stmt;
    sql::ResultSet*  res;

    driver = get_driver_instance();

    // con = driver->connect(HOSTNAME_LOCAL, USER_LOCAL, PWD_LOCAL);
    con = driver->connect(HOSTNAME, USER, PWD);

    con->setSchema("dialogue2020");

    stmt = con->createStatement();
    res  = stmt->executeQuery("SELECT 'Hello World!' AS _message");

    while (res->next()) {
        std::cout << "\t... MySQL replies: ";
        std::cout << res->getString("_message") << std::endl;
    }

    delete res;
    delete stmt;
    delete con;

    std::cout << "done!" << std::endl;
} catch (sql::SQLException& e) {
    std::cout << "# ERR: SQLException in " << __FILE__;
    std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
    std::cout << "# ERR: " << e.what();
    std::cout << " (MySQL error code: " << e.getErrorCode();
    std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
} catch (std::exception& ex) {
    std::cout << "STD EXCEPTION: " << ex.what() << std::endl;
    return 1;
} catch (const char* ex) {
    std::cout << "EXCEPTION: " << ex << std::endl;
    return 1;
}