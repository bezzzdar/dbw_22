#include <tgbot/tgbot.h>
#include <tgbot/TgException.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <sstream>
#include <vector>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bot_utils.h"
#include "db_api.h"

#include "parser.h"
#include "lexer.h"
#include "instruction.h"

// ====================
// TYPES

// user's possible states.
// NOTE: needs it's own state machine to avoid illegal states and allow for clearer state changes
enum BotState {
    NO_STATE,             // current user is unknown
    REGISTERING_NAME,     // waiting for name input
    REGISTERING_SCHOOL,   // waiting for school number  
    REGISTERING_GRADE,    // waiting for grade number  
    WAITING,              // just waiting
    NO_DISCIPLINE_CHOSEN, // waiting for discipline choise
    PHY_CHOSEN,           // waiting for answer
    BIO_CHOSEN,           // waiting for answer
    GEO_CHOSEN,           // waiting for answer
    COD_CHOSEN,           // waiting for answer
    HIST_CHOSEN,          // waiting for answer
    CHEM_CHOSEN,          // waiting for answer
    ENG_CHOSEN,           // waiting for answer
    CULT_CHOSEN,          // waiting for answer
    MATH_CHOSEN,          // waiting for answer
    SOCIAL_CHOSEN,        // waiting for answer
    LISTENING_ADMIN,      // waiting for admin command
    GET_COMMAND,          // get command and parsing/executing it
 };

typedef std::map<db_api::Disciplines, std::list<size_t>> TasksStack;

// information about each user. can be broadened if needed
struct UserInfo {
    std::string name = "";
    int         school = -1;
    int         grade = -1;

    int category = -1;  // 0 - secondarySchool, 1 - high school

    BotState state = NO_STATE;

    int user_id = -1;
    int user_score = 0;

    TasksStack tasks_stack;
};

int AllAnswers = 0;
int RightAnswers = 0;

// ====================
// FUNC DECLARATION

std::stringstream SendNumberOfAnswers(int& AllAnswers, int& RightAnswers, db_api::Connector& conn);
void InitTasksStack(TasksStack* stack, db_api::Connector& conn, int grade);
void ReadUserInfo(const std::string& path_to_save);
void SerializeUserInfo();
void CalculateResults();
void SigHandler(int s);

void Logic(const Ins &i,db_api::Connector& conn);

BotState IntToBotState(int number);

void CreateBackupUserInfo(int category);
void ReadFromBackupUserInfo(int category);
void SetStateToUsers(int newState, int category);

// ====================
// GLOBALS

const char*             BOT_TOKEN = "";
std::map<int, UserInfo> CHAT_ID_TO_USER_INFO{};
std::map<int, UserInfo> CHAT_ID_TO_USER_INFO_BACKUP{};
Parser pars;
std::pair<Ins, Error> res;
std::stringstream replyForCommand;
auto currentTime = std::chrono::system_clock::now();
std::time_t sendTime; // = std::chrono::system_clock::to_time_t(currentTime);

// ====================
// MAIN

//Prepods chat id
//<254764569> - Алина

int main(int argc, char* argv[]) {
    currentTime = std::chrono::system_clock::now();
    sendTime = std::chrono::system_clock::to_time_t(currentTime);
    
    // back-up data storage
    // on unhandled exception
    std::set_terminate([]() {
        std::cout << "Unhandled exception or abort occured!\n" << std::endl;

        SerializeUserInfo();

        std::abort();
    });

    // on normal termination
    std::atexit([]() {
        std::cout << "Bot normally terminated!\n" << std::endl;

        SerializeUserInfo();

        CalculateResults();
    });

    // on signal. if bot terminates unexpectedly, it will serialize user's data
    struct sigaction action_sigint;
    struct sigaction action_sigterm;

    action_sigint.sa_handler = SigHandler;
    sigemptyset(&action_sigint.sa_mask);
    action_sigint.sa_flags = 0;

    action_sigterm.sa_handler = SigHandler;
    sigemptyset(&action_sigterm.sa_mask);
    action_sigterm.sa_flags = 0;

    sigaction(SIGINT, &action_sigint, NULL);
    sigaction(SIGTERM, &action_sigterm, NULL);

    // no-buffer output
    std::cout << std::unitbuf;

    // arg parsing

    // parameters of the DB
    const std::string hostname(argv[1]);
    const std::string username(argv[2]);
    const std::string password(argv[3]);

    // const std::string username = "root";
    // const std::string hostname = "tcp://LAPTOP-E950M0TH:3306";
    // const std::string password = "****";

    const std::string path_to_save = (argc == 5) ? (std::string(argv[4])) : (std::string("chat_id_to_u_info.log"));

    // path initialization
    std::string path_to_pics{argv[0]};

    const auto iter_dir = path_to_pics.rfind('/');
    path_to_pics = path_to_pics.substr(0, (iter_dir == std::string::npos) ? (0) : (iter_dir + 1));
    path_to_pics += "../../db/";

    std::cout << "path to pics: <" << path_to_pics << ">\n";

    // CHAT_ID_TO_USER_INFO initialization
    if (!path_to_save.empty()) {
        std::cout << "Reading user info from " << path_to_save << "\n";

        // TODO: error handling
        ReadUserInfo(path_to_save);

        std::cout << "Successfully read user data\n";
    }

    // main entities
    db_api::Connector conn(hostname.c_str(), username.c_str(), password.c_str(), "dialogue2022");

    TgBot::Bot bot(BOT_TOKEN);

    // setting up telegram keyboards    
    
    TgBot::InlineKeyboardMarkup::Ptr disciplines_keyboard(new TgBot::InlineKeyboardMarkup);
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row0;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row1;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row2;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row3;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row4;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row5;

    TgBot::InlineKeyboardButton::Ptr phy(new TgBot::InlineKeyboardButton);
    phy->text = "физика";
    phy->callbackData = "phy";

    TgBot::InlineKeyboardButton::Ptr bio(new TgBot::InlineKeyboardButton);
    bio->text = "биология";
    bio->callbackData = "bio";

    TgBot::InlineKeyboardButton::Ptr geo(new TgBot::InlineKeyboardButton);
    geo->text = "география";
    geo->callbackData = "geo";

    TgBot::InlineKeyboardButton::Ptr cod(new TgBot::InlineKeyboardButton);
    cod->text = "информатика";
    cod->callbackData = "cod";

    TgBot::InlineKeyboardButton::Ptr hist(new TgBot::InlineKeyboardButton);
    hist->text = "история";
    hist->callbackData = "hist";

    TgBot::InlineKeyboardButton::Ptr chem(new TgBot::InlineKeyboardButton);
    chem->text = "химия";
    chem->callbackData = "chem";

    TgBot::InlineKeyboardButton::Ptr eng(new TgBot::InlineKeyboardButton);
    eng->text = "английский";
    eng->callbackData = "eng";

    TgBot::InlineKeyboardButton::Ptr cult(new TgBot::InlineKeyboardButton);
    cult->text = "культура";
    cult->callbackData = "cult";

    TgBot::InlineKeyboardButton::Ptr math(new TgBot::InlineKeyboardButton);
    math->text = "математика";
    math->callbackData = "math";

    TgBot::InlineKeyboardButton::Ptr social(new TgBot::InlineKeyboardButton);
    social->text = "обществознание";
    social->callbackData = "social";

    disciplines_row0.push_back(phy);
    disciplines_row0.push_back(bio);
    disciplines_row1.push_back(geo);
    disciplines_row1.push_back(cod);
    disciplines_row2.push_back(hist);
    disciplines_row2.push_back(chem);
    disciplines_row3.push_back(cult);
    disciplines_row3.push_back(math);
    disciplines_row4.push_back(eng);
    disciplines_row5.push_back(social);

    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row0);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row1);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row2);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row3);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row4);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row5);

    TgBot::InlineKeyboardMarkup::Ptr              tasks_keyboard(new TgBot::InlineKeyboardMarkup);
    std::vector<TgBot::InlineKeyboardButton::Ptr> tasks_row0;

    TgBot::InlineKeyboardButton::Ptr next_button(new TgBot::InlineKeyboardButton);
    next_button->text = "другой вопрос";
    next_button->callbackData = "next_button";

    TgBot::InlineKeyboardButton::Ptr choose_button(new TgBot::InlineKeyboardButton);
    choose_button->text = "выбрать тему";
    choose_button->callbackData = "choose_button";

    tasks_row0.push_back(next_button);
    tasks_row0.push_back(choose_button);

    tasks_keyboard->inlineKeyboard.push_back(tasks_row0);

    // bot "/start" handler
        bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        printf("onCommand start\n");
        const auto chat_id = message->chat->id;
        
        std::stringstream reply;

         if(chat_id == 194750541)
        {
          reply << "Слушаю и повинуюсь, моя госпожа\n";
          bot.getApi().sendMessage(chat_id, reply.str());          
          CHAT_ID_TO_USER_INFO[chat_id].state = BotState::LISTENING_ADMIN;
        }
        // if (chat_id == 118782290)
        // {
        //     reply << "О, Великий!\n";
        // }
       
        try{   
            if (CHAT_ID_TO_USER_INFO[chat_id].state < BotState::REGISTERING_NAME) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::REGISTERING_NAME;

                reply << "Привет! Скажи имя, под которым ты хочешь, чтобы я тебя "
                     "зарегистрировал в формате Имя Фамилия, пожалуйста\n";
                std::cout << "send hi message to user in chat <" << chat_id <<"> \n";     

                bot.getApi().sendMessage(chat_id, reply.str());
            } else {
            reply << "Жду ввода имени...\n";
            std::cout << "wait to name of user in chat <" << chat_id <<"> \n";

            bot.getApi().sendMessage(chat_id, reply.str());
            }
        }
        catch (const std::runtime_error& re) {
            std::cerr << "Runtime error: " << re.what() << std::endl;
            std::cout << "Runtime error on starting registration chat_id: " << chat_id << "\n";
        } catch (const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
        std::cout << "Error on statring registration chat_id: " << chat_id << "\n";
        }

        });
    
    
    // bot additional commands
    bot.getEvents().onCommand("exit", [&bot, &conn](TgBot::Message::Ptr message) {
        const auto chat_id = message->chat->id;

        std::stringstream reply;

        //CHAT_ID_TO_USER_INFO.erase(chat_id);

        reply << "Пока! У тебя правильно решено "
              << conn.RequestUserScore(CHAT_ID_TO_USER_INFO[chat_id].user_id)
              << " заданий. Этот результат будет сохранен в нашей "
                 "базе данных под твоим именем.\n";
        

        CHAT_ID_TO_USER_INFO.erase(chat_id);

        bot.getApi().sendMessage(chat_id, reply.str());
    });

    // bot logic on user button input
    bot.getEvents().onCallbackQuery([&bot,
                                     &tasks_keyboard,
                                     &disciplines_keyboard,
                                     &conn,
                                     &path_to_pics](TgBot::CallbackQuery::Ptr query) {
        // TODO: add code generation / define to avoid this copy-paste
        std::string query_data = query->data;
        const auto  chat_id = query->message->chat->id;

        auto       user_info = CHAT_ID_TO_USER_INFO[chat_id];
        const bool can_choose_discipline = user_info.state >= BotState::NO_DISCIPLINE_CHOSEN;
        const bool is_answering_question = user_info.state > BotState::NO_DISCIPLINE_CHOSEN;

        std::stringstream reply;

        if (can_choose_discipline) {
            db_api::Disciplines discipline = db_api::Disciplines::NONE;

            if (StringTools::startsWith(query_data, "phy")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::PHY_CHOSEN;

                reply << "Раздел физика:\n";

                discipline = db_api::Disciplines::PHY;
            } else if (StringTools::startsWith(query_data, "bio")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::BIO_CHOSEN;

                reply << "Раздел биология:\n";

                discipline = db_api::Disciplines::BIO;
            } else if (StringTools::startsWith(query_data, "geo")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::GEO_CHOSEN;

                reply << "Раздел география:\n";

                discipline = db_api::Disciplines::GEO;
            } else if (StringTools::startsWith(query_data, "cod")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::COD_CHOSEN;

                reply << "Раздел информатика:\n";

                discipline = db_api::Disciplines::COD;
            } else if (StringTools::startsWith(query_data, "hist")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::HIST_CHOSEN;

                reply << "Раздел история:\n";

                discipline = db_api::Disciplines::HIST;
            } else if (StringTools::startsWith(query_data, "chem")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::CHEM_CHOSEN;

                reply << "Раздел химия:\n";

                discipline = db_api::Disciplines::CHEM;
            } else if (StringTools::startsWith(query_data, "cult")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::CULT_CHOSEN;

                reply << "Раздел культура:\n";

                discipline = db_api::Disciplines::CULT;
            } else if (StringTools::startsWith(query_data, "math")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::MATH_CHOSEN;

                reply << "Раздел математика:\n";

                discipline = db_api::Disciplines::MATH;
            } else if (StringTools::startsWith(query_data, "eng")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::ENG_CHOSEN;

                reply << "Раздел английский:\n";

                discipline = db_api::Disciplines::ENG;
            } else if (StringTools::startsWith(query_data, "social")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::SOCIAL_CHOSEN;

                reply << "Раздел обществознание:\n";

                discipline = db_api::Disciplines::SOCIAL;
            }

            if (discipline != db_api::Disciplines::NONE) {
                bool is_depleted = user_info.tasks_stack[discipline].empty();

                if (!is_depleted) {
                    const auto task =
                        conn.RequestTask(discipline, user_info.tasks_stack[discipline].front(), user_info.grade);

                    reply << task.text << '\n';
                    currentTime = std::chrono::system_clock::now();
                    sendTime = std::chrono::system_clock::to_time_t(currentTime);

                    std::cout << "At time " << std::ctime(&sendTime) << " sending text: <" << task.text << "> to user " << user_info.name << " at chat <" << chat_id << ">\n";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);

                    if (!task.pic_name.empty()) {
                        std::string path_to_pic = path_to_pics +
                                                  db_api::discipline_to_string.at(discipline) +
                                                  '/' + task.pic_name;

                        std::string mime_type = "image/";

                        const auto iter_dir = path_to_pic.rfind('.');
                        mime_type += path_to_pic.substr(iter_dir + 1);

                        std::cout << "At time " << std::ctime(&sendTime) << " sending photo 1: <" << path_to_pic << "> <" << mime_type
                                  << "> to user " << user_info.name << " at chat <" << chat_id << ">\n";

                        try {
                            bot.getApi().sendPhoto(
                                chat_id, TgBot::InputFile::fromFile(path_to_pic, mime_type));
                        } catch (TgBot::TgException& tgex) {
                            std::cout << "At time " << std::ctime(&sendTime) << " error sending photo to user <" << user_info.name << "> at chat <" << chat_id << ">\n";
                            std::cout << tgex.what() << std::endl;

                            reply.str("");

                            reply << "Извини, произошла ошибка при отправке фото к этому заданию. "
                                     "Порешай пока другие и через какое то время попытайся "
                                     "вернуться к этому. Прости(\n";

                            bot.getApi().sendMessage(chat_id, reply.str());
                        } catch (const std::runtime_error& re) {
                            std::cout << "At time " << std::ctime(&sendTime) << " runtime error sending photo to user <" << user_info.name << "> at chat <" << chat_id << ">\n";
                            std::cout << re.what() << std::endl;

                            reply.str("");

                            reply << "Если ты видишь это сообщение, значит "
                                     "фото не отправилось. Порешай пока другие "
                                     "задания и через какое то время попытайся вернуться к этому. "
                                     "Прости(\n";

                            bot.getApi().sendMessage(chat_id, reply.str());
                        }
                    }
                } else {
                    CHAT_ID_TO_USER_INFO[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                    reply << "К сожалению, больше вопросов в этой категории нет, выбери другую, "
                             "пожалуйста\n";
                    std::cout << "At time " << std::ctime(&sendTime) << " send end_of_discipline_message to user <" << user_info.name << "> at chat <" << chat_id << ">\n";
                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
                }
            }
        }

        if (is_answering_question) {
            db_api::Disciplines discipline = db_api::Disciplines::NONE;

            switch (user_info.state) {
            case BotState::PHY_CHOSEN:
                discipline = db_api::Disciplines::PHY;
                break;
            case BotState::MATH_CHOSEN:
                discipline = db_api::Disciplines::MATH;
                break;
            case BotState::GEO_CHOSEN:
                discipline = db_api::Disciplines::GEO;
                break;
            case BotState::BIO_CHOSEN:
                discipline = db_api::Disciplines::BIO;
                break;
            case BotState::COD_CHOSEN:
                discipline = db_api::Disciplines::COD;
                break;
            case BotState::CULT_CHOSEN:
                discipline = db_api::Disciplines::CULT;
                break;
            case BotState::ENG_CHOSEN:
                discipline = db_api::Disciplines::ENG;
                break;
            case BotState::HIST_CHOSEN:
                discipline = db_api::Disciplines::HIST;
                break;
            case BotState::CHEM_CHOSEN:
                discipline = db_api::Disciplines::CHEM;
                break;
            case BotState::SOCIAL_CHOSEN:
                discipline = db_api::Disciplines::SOCIAL;
                break;
            default:
                return;
            }

            if (StringTools::startsWith(query_data, "next_button")) {
                if (user_info.tasks_stack[discipline].empty()) {
                    return;
                }

                reply << "Ок, вот следующий вопрос:\n";

                CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].push_back(
                    user_info.tasks_stack[discipline].front());

                CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].pop_front();

                const auto task = conn.RequestTask(
                    discipline, CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].front(), CHAT_ID_TO_USER_INFO[chat_id].grade);

                reply << task.text << '\n';
                
                auto currentTime = std::chrono::system_clock::now();
                std::time_t sendTime = std::chrono::system_clock::to_time_t(currentTime);

                std::cout << "sending text: <" << task.text << "> to user " << user_info.name << " at time " << std::ctime(&sendTime) <<"\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);

                if (!task.pic_name.empty()) {
                    std::string path_to_pic = path_to_pics +
                                              db_api::discipline_to_string.at(discipline) + '/' +
                                              task.pic_name;

                    std::string mime_type = "image/";

                    const auto iter_dir = path_to_pic.rfind('.');
                    mime_type += path_to_pic.substr(iter_dir + 1);

                    std::cout << "sending photo 2: <" << path_to_pic << "> <" << mime_type << "> to user " << user_info.name << " at time " << std::ctime(&sendTime) <<"\n";

                    try {
                        bot.getApi().sendPhoto(chat_id,
                                               TgBot::InputFile::fromFile(path_to_pic, mime_type));
                    } catch (TgBot::TgException& tgex) {
                        std::cout << "error sending photo to user " << user_info.name << " at time " << std::ctime(&sendTime) <<"\n";
                        std::cout << tgex.what() << std::endl;

                        reply.str("");

                        reply << "Извини, произошла ошибка при отправке фото к этому заданию. "
                                 "Порешай пока другие и через какое то время попытайся "
                                 "вернуться к этому. Прости(\n";

                        bot.getApi().sendMessage(chat_id, reply.str());
                    } catch (const std::runtime_error& re) {
                        std::cout << "runtime error sending photo:\n";
                        std::cout << re.what() << std::endl;

                        reply.str("");

                        reply << "Если ты видишь это сообщение, значит "
                                 "фото не отправилось. Порешай пока другие задания и "
                                 "через какое-то время попытайся вернуться к этому. Прости(\n";

                        bot.getApi().sendMessage(chat_id, reply.str());
                    }
                }
            } else if (StringTools::startsWith(query_data, "choose_button")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                reply << "Хорошо, вот темы на выбор:\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
            }
        }

        return;
    });
    printf("*\n");
    // bot logic on any non-command message
    bot.getEvents().onNonCommandMessage([&bot,
                                         &conn,
                                         &disciplines_keyboard,
                                         &tasks_keyboard,
                                         &path_to_pics](const TgBot::Message::Ptr& message) {
        const auto chat_id = message->chat->id;
        auto       message_text = message->text;
        std::string command;
        printf("*обработка NonCommandMessage\n");

        auto       user_info = CHAT_ID_TO_USER_INFO[chat_id];
        const auto user_id = user_info.user_id;

        std::cout << "user <" << user_info.name << "> in chat " << chat_id << " wrote:\n<"
                  << message_text << "> user state: <" <<user_info.state <<">\n";

        

        db_api::Disciplines discipline = db_api::Disciplines::NONE;

        std::stringstream reply;

        // large switch-case for each user's state
        // TODO: if user state is to be remade with it's own state machine, this swould be the part
        // of the machine, something llike "enumerate_state" and each state change should be done
        // via this stete machine
        switch (CHAT_ID_TO_USER_INFO[chat_id].state) {
        case NO_STATE:
            
            try{
                bot.getApi().sendMessage(chat_id, "Чтобы начать, введи команду /start, пожалуйста\n");
                std::cout << "send start message to user in chat <" << chat_id << ">\n";
                }
            catch (const std::runtime_error& re) {
            std::cerr << "Runtime error: " << re.what() << std::endl;
            std::cout << "упало при попытке отправить старт chat_id: " << chat_id << "\n";
        } catch (const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
        std::cout << "упало на старте chat_id: " << chat_id << "\n";
        }            
            break;
        case REGISTERING_NAME:
            bool is_duplicate;
            is_duplicate = conn.UsernameTaken(message_text);

            if (is_duplicate) {
                reply << "Так вышло, что человека с таким именем уже зарегистрировали. Обратись к "
                         "организаторам, пожалуйста\n";
                std::cout << "we have duplicate name in chat <" << chat_id << ">\n";         
            } else if (bot_utils::IsValidName(message_text)) {
                reply << "Привет, " << message_text
                      << "\nТеперь введи номер своей школы. Только цифру, пожалуйста\n"
                         "Если в названии школы не только цифра, организаторы присвоили этой школе "
                         "какой-то номер, спроси у них, какой\n";
                std::cout << "name of user in chat <" << chat_id << ">  is valid\n";
                CHAT_ID_TO_USER_INFO[chat_id].name = bot_utils::ToLowerNoSpaces(message_text);
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::REGISTERING_SCHOOL;
            } else {
                reply << "Пожалуйста, проверь, что всё введено корректно.\n";
                std::cout << "name of user in chat <" << chat_id << ">  is not valid\n";
            };
            try
            {
                bot.getApi().sendMessage(chat_id, reply.str());
                std::cout << "send reply about name <" << chat_id << ">  is valid or not valid\n";
            }
            catch (const std::runtime_error& re) {
                std::cerr << "Runtime error: " << re.what() << std::endl;
                std::cout << "fail on reply on registering_name state to chat_id: " << chat_id << "\n";
            } catch (const std::exception& ex) {
                std::cerr << "Error occurred: " << ex.what() << std::endl;
                std::cout << "fail on reply on registering_name state to chat_id: " << chat_id << "\n";
            }
            

            break;
            
        case REGISTERING_SCHOOL:
            int  school_n;
            bool is_valid_n;

            try {
                
                school_n = std::stoi(message_text);

                is_valid_n = bot_utils::IsValidSchool(school_n);

                if (!is_valid_n) {
                    reply << "Уверен, школы с таким номером нет\n";
                }
            } catch (const std::invalid_argument& inv_arg) {
                reply << "Пожалуйста, введи только номер школы. Только цифры\n";

                is_valid_n = false;
            } catch (const std::out_of_range& oor) {
                reply << "Столько школ во всем мире не наберется\n";

                is_valid_n = false;
            }

            if (is_valid_n) {
                reply << "Здорово, теперь введи номер своего класса. Только цифру, пожалуйста.\n ";

                CHAT_ID_TO_USER_INFO[chat_id].school = school_n;
            //  CHAT_ID_TO_USER_INFO[chat_id].user_id = conn.AddUser(user_info.name, school_n);
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::REGISTERING_GRADE;

            //   InitTasksStack(&CHAT_ID_TO_USER_INFO[chat_id].tasks_stack, conn);

            //   reply << "Теперь выбери, какие вопросы хочешь решать. Категорию можно "
            //            "изменить в любой момент, так что не бойся экспериментировать\n";

                bot.getApi().sendMessage(chat_id, reply.str());
            } else {
                bot.getApi().sendMessage(chat_id, reply.str());
            }
            break;

        case REGISTERING_GRADE:
            int grade_n;
            bool is_valid_g;
            
            try {
            
                grade_n = std::stoi(message_text);

                is_valid_g = bot_utils::IsValidGrade(grade_n);

                if (!is_valid_g) {
                    reply << "Уверен, класса с таким номером нет\n";
                }
            } catch (const std::invalid_argument& inv_arg) {
                reply << "Пожалуйста, введи только номер класса. Только цифры\n";

                is_valid_g = false;
            }
            catch (const std::out_of_range& oor) {
                reply << "Столько классов в школе не бывает\n";

                is_valid_g = false;
            }

            if (is_valid_g) {
                reply << "Здорово, ты успешно зарегистрирован(а) как ученик " << grade_n << " класса школы № " << user_info.school
                      << "\n";

                CHAT_ID_TO_USER_INFO[chat_id].grade = grade_n;
                if(grade_n <= 9) CHAT_ID_TO_USER_INFO[chat_id].category=0;
                else CHAT_ID_TO_USER_INFO[chat_id].category=1;
                CHAT_ID_TO_USER_INFO[chat_id].user_id = conn.AddUser(user_info.name, user_info.school, grade_n);
                //CHAT_ID_TO_USER_INFO[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::WAITING;
                InitTasksStack(&CHAT_ID_TO_USER_INFO[chat_id].tasks_stack, conn, grade_n);

                reply << "Теперь ты можешь пообщаться с людьми вокруг и подождать, пока преподы запустят игру\n";
                //reply << "Теперь выбери, какие вопросы хочешь решать. Категорию можно "
                //         "изменить в любой момент, так что не бойся экспериментировать\n";

                bot.getApi().sendMessage(chat_id, reply.str());

                //bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
            } else {
                bot.getApi().sendMessage(chat_id, reply.str());
            }

            break;
        case WAITING:
            reply << "Пожалуйста, подожди, пока админы начнут игру\n";
            bot.getApi().sendMessage(chat_id, reply.str());
            std::cout << "At time " << std::ctime(&sendTime) << " send waiting message to user " << user_info.name << " at chat <" << chat_id << "> " << IntToBotState(user_info.state) << "\n";

            break;
        case NO_DISCIPLINE_CHOSEN:
            reply << "Жмякни на кнопку с интересующей тебя категорией, пожалуйста\n";

            bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
            break;
        case PHY_CHOSEN:
            discipline = db_api::Disciplines::PHY;
            break;
        case BIO_CHOSEN:
            discipline = db_api::Disciplines::BIO;
            break;
        case GEO_CHOSEN:
            discipline = db_api::Disciplines::GEO;
            break;
        case COD_CHOSEN:
            discipline = db_api::Disciplines::COD;
            break;
        case HIST_CHOSEN:
            discipline = db_api::Disciplines::HIST;
            break;
        case CHEM_CHOSEN:
            discipline = db_api::Disciplines::CHEM;
            break;
        case CULT_CHOSEN:
            discipline = db_api::Disciplines::CULT;
            break;
        case ENG_CHOSEN:
            discipline = db_api::Disciplines::ENG;
            break;
        case MATH_CHOSEN:
            discipline = db_api::Disciplines::MATH;
            break;
        case SOCIAL_CHOSEN:
            discipline = db_api::Disciplines::SOCIAL;
            break;
        case LISTENING_ADMIN:
            reply << "готов воспринимать команды \n";
            bot.getApi().sendMessage(chat_id, reply.str());
            CHAT_ID_TO_USER_INFO[chat_id].state = GET_COMMAND;
        break;
        case GET_COMMAND:   
            res = pars.Parse(message_text);
            Logic(res.first, conn);
            bot.getApi().sendMessage(194750541, replyForCommand.str());            
        break;
        default:
            discipline = db_api::Disciplines::PHY;
            break;
        }
        
        //parsing command 
        
        
        // checking answer
        if (discipline != db_api::Disciplines::NONE) {
            bool ans_is_correct = conn.CheckAnswer(
                message_text, discipline, user_info.tasks_stack[discipline].front(), user_info.grade);

            if (ans_is_correct) {
                const auto task_id = CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].front();

                conn.RegisterCorrectAnswer(user_id, discipline, task_id, user_info.grade);

                CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].pop_front();
                CHAT_ID_TO_USER_INFO[chat_id].user_score++;

                reply << "Ответ правильный, молодец!\n";

                if (!CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].empty()) {
                    reply << "Вот следующее задание:\n";

                    const auto task = conn.RequestTask(
                        discipline, CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].front(), CHAT_ID_TO_USER_INFO[chat_id].grade);

                    reply << task.text << '\n';

                    std::cout << "sending text: <" << task.text << ">\n";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);

                    if (!task.pic_name.empty()) {
                        std::string path_to_pic = path_to_pics +
                                                  db_api::discipline_to_string.at(discipline) +
                                                  '/' + task.pic_name;

                        std::string mime_type = "image/";

                        const auto iter_dir = path_to_pic.rfind('.');
                        mime_type += path_to_pic.substr(iter_dir + 1);

                        std::cout << "sending photo 3: <" << path_to_pic << "> <" << mime_type
                                  << ">\n";

                        try {
                            bot.getApi().sendPhoto(
                                chat_id, TgBot::InputFile::fromFile(path_to_pic, mime_type));
                        } catch (TgBot::TgException& tgex) {
                            std::cout << "error sending photo:\n";
                            std::cout << tgex.what() << std::endl;

                            reply.str("");

                            reply << "Извини, произошла ошибка при отправке фото к этому заданию. "
                                     "Порешай пока другие и через какое то время попытайся "
                                     "вернуться к этому. Прости(\n";

                            bot.getApi().sendMessage(chat_id, reply.str());
                        } catch (const std::runtime_error& re) {
                            std::cout << "runtime error sending photo:\n";
                            std::cout << re.what() << std::endl;

                            reply.str("");

                            reply << "Если ты видишь это сообщение, значит "
                                     "фото не отправилось. Порешай пока другие "
                                     "задания и "
                                     "через какое то время попытайся вернуться к этому. Прости(\n";

                            bot.getApi().sendMessage(chat_id, reply.str());
                        }
                    }
                
                } else {
                    reply << "Больше вопросов в этой категории нет. Но ты всегда можешь "
                             "попробовать другие)\n";

                    CHAT_ID_TO_USER_INFO[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
                }

            } else {
                reply << "Ответ неправильный. Попытайся еще раз\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);
            }
        }

        return;
    });

    try {
        std::cout << "Bot username: " << bot.getApi().getMe()->username.c_str() << "\n";
        }
    catch (const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    }

    TgBot::TgLongPoll longPoll(bot);

    try {
        while (true) {
            std::cout << "Long poll started\n";

            longPoll.start();
        }
    } catch (const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;
    }




    return 0;
}

// ====================
// FUNC IMPLEMENTATION

void SetStateToUsers(int newState, int category)
{
    CreateBackupUserInfo(category);
    for(auto& user : CHAT_ID_TO_USER_INFO)
    {
        if(user.second.category == category) user.second.state = (BotState)newState;
    }
    return;
}

void ReturnUserState(int category)
{
    ReadFromBackupUserInfo(category);   
}

BotState IntToBotState(int number)
{
    return BotState(number);

}

void SetStateToSecondarySchool(BotState newState)
{     
    for(auto & user : CHAT_ID_TO_USER_INFO)
    {
        if(user.second.grade <= 9)
        {
            user.second.state = newState;
        }
    }
    return;
}

void CreateBackupUserInfo(int category)
{
    for(auto & user : CHAT_ID_TO_USER_INFO)
    {
        if (user.second.category==category)
        {
            CHAT_ID_TO_USER_INFO_BACKUP[user.first] = CHAT_ID_TO_USER_INFO[user.first];
            CHAT_ID_TO_USER_INFO_BACKUP[user.second.state]= CHAT_ID_TO_USER_INFO[user.second.state];
        }
    }
    return;
}

void ReadFromBackupUserInfo(int category)
{
    for(auto & user : CHAT_ID_TO_USER_INFO_BACKUP)
    {
        if (user.second.category==category)
        {
        CHAT_ID_TO_USER_INFO[user.first] = CHAT_ID_TO_USER_INFO_BACKUP[user.first];
        CHAT_ID_TO_USER_INFO[user.second.state]= CHAT_ID_TO_USER_INFO_BACKUP[user.second.state];
        }
    }

    return;
}

std::stringstream SendNumberOfAnswers(int& AllAnswers, int& RightAnswers, db_api::Connector& conn)
{
    std::stringstream reply;
    conn.NumberAnswers(AllAnswers, RightAnswers);
    reply << "ответов всего: " << AllAnswers << ".\n"
          << "правильных ответов: " << RightAnswers << ".\n";
    
    return reply;
}

void InitDiscipline(std::list<size_t>* tasks, const size_t n_tasks) {
    for (size_t i = 1; i <= n_tasks; i++) {
        tasks->push_back(i);
    }

    return;
}

void InitTasksStack(TasksStack* stack, db_api::Connector& conn, int grade) {
    InitDiscipline(&((*stack)[db_api::Disciplines::PHY]),
                   conn.RequestNumberTasks(db_api::Disciplines::PHY, grade));
    InitDiscipline(&((*stack)[db_api::Disciplines::MATH]),
                   conn.RequestNumberTasks(db_api::Disciplines::MATH, grade));
    InitDiscipline(&((*stack)[db_api::Disciplines::GEO]),
                   conn.RequestNumberTasks(db_api::Disciplines::GEO, 0));
    InitDiscipline(&((*stack)[db_api::Disciplines::BIO]),
                   conn.RequestNumberTasks(db_api::Disciplines::BIO, grade));
    InitDiscipline(&((*stack)[db_api::Disciplines::COD]),
                   conn.RequestNumberTasks(db_api::Disciplines::COD, grade));
    InitDiscipline(&((*stack)[db_api::Disciplines::CULT]),
                   conn.RequestNumberTasks(db_api::Disciplines::CULT, 0));
    InitDiscipline(&((*stack)[db_api::Disciplines::HIST]),
                   conn.RequestNumberTasks(db_api::Disciplines::HIST, grade));
    InitDiscipline(&((*stack)[db_api::Disciplines::CHEM]),
                   conn.RequestNumberTasks(db_api::Disciplines::CHEM, grade));
    InitDiscipline(&((*stack)[db_api::Disciplines::ENG]),
                   conn.RequestNumberTasks(db_api::Disciplines::ENG, grade));
    InitDiscipline(&((*stack)[db_api::Disciplines::SOCIAL]),
                   conn.RequestNumberTasks(db_api::Disciplines::SOCIAL, 0));

    return;
}

void ReadUserInfo(const std::string& path_to_save) {
    std::ifstream f(path_to_save);

    CHAT_ID_TO_USER_INFO.clear();

    for (std::string line; std::getline(f, line);) {
        const auto tokens(bot_utils::Parse(line, ';'));

        if (tokens.size() == 0) {
            continue;
        }

        UserInfo u_info{};

        const auto chat_id = std::stoi(tokens[0]);
        u_info.name = tokens[1];
        u_info.school = std::stoi(tokens[2]);
        u_info.state = static_cast<BotState>(std::stoi(tokens[3]));
        u_info.user_id = std::stoi(tokens[4]);
        u_info.user_score = std::stoi(tokens[5]);

        for (size_t i = 6; i < tokens.size(); i++) {
            const auto task_stack(bot_utils::Parse(tokens[i], ','));

            const auto discipline = static_cast<db_api::Disciplines>(std::stoi(task_stack[0]));

            std::list<size_t> tasks;
            for (size_t j = 1; j < task_stack.size(); j++) {
                tasks.push_back(static_cast<size_t>(std::stoi(task_stack[j])));
            }

            u_info.tasks_stack.emplace(std::make_pair(discipline, std::move(tasks)));
        }

        CHAT_ID_TO_USER_INFO.emplace(std::make_pair(chat_id, std::move(u_info)));
    }

    f.close();

    return;
}

void SerializeUserInfo() {
    std::cout << "> serializing...\n";

    std::ofstream f;
    f.open("chat_id_to_u_info.log", std::ofstream::out | std::ofstream::trunc);

    for (const auto& pair_info : CHAT_ID_TO_USER_INFO) {
        f << pair_info.first << ';';
        f << pair_info.second.name << ';' << pair_info.second.school << ';'
          << pair_info.second.state << ';' << pair_info.second.user_id << ';'
          << pair_info.second.user_score;

        for (const auto& pair_stack : pair_info.second.tasks_stack) {
            f << ';' << pair_stack.first;

            for (const auto& task_id : pair_stack.second) {
                f << ',' << task_id;
            }
        }

        f << '\n';
    }

    f.close();

    std::cout << "> done!\n";

    return;
}

void CalculateResults() {
    std::cout << "calculating results...\n";

    std::map<int, int>                       points_per_school{};
    std::pair<std::vector<std::string>, int> winner{};

    // std::vector<std::pair<std::vector<std::string>, int>> winners{};

    std::ofstream f;
    f.open("results.log", std::ofstream::out | std::ofstream::trunc);

    if (!f.is_open()) {
        std::cout << "not open!\n";
    }

    for (const auto& pair : CHAT_ID_TO_USER_INFO) {
        if (pair.second.user_score > winner.second) {
            winner.first.clear();

            winner.first.push_back(pair.second.name);
            winner.second = pair.second.user_score;
        } else if (pair.second.user_score == winner.second) {
            winner.first.push_back(pair.second.name);
        }

        points_per_school[pair.second.school] += pair.second.user_score;
    }

    // for (size_t i = 0; i < 5; i++) {
    //     for (const auto& pair : CHAT_ID_TO_USER_INFO) {
    //         for (size_t j = 0; j < i; j++) {
    //             if (pair.second.user_score >= winners[j].second) {
    //                 continue;
    //             }
    //         }

    //         if (pair.second.user_score > winners[i].second) {
    //             winners[i].first.clear();

    //             winners[i].first.push_back(pair.second.name);
    //             winners[i].second = pair.second.user_score;
    //         } else if (pair.second.user_score == winners[i].second) {
    //             winners[i].first.push_back(pair.second.name);
    //         }
    //     }
    // }

    f << "winner user(s):\n";
    for (const auto& name : winner.first) {
        f << '\t' << '<' << name << '>' << '\n';
    }
    // for (const auto& token : winners) {
    //     f << '\t' << std::setw(10) << std::setfill('.') << std::left << token.second << ":";

    //     for (const auto& name : token.first) {
    //         f << '<' << name << '>';
    //     }

    //     f << '\n';
    // }

    f << "\nschools:\n";
    for (const auto school : points_per_school) {
        f << '\t' << std::setw(10) << std::setfill('.') << std::left << school.first << ":"
          << school.second << '\n';
    }

    f.close();

    return;
}

void SigHandler(int s) {
    printf("Caught signal %d\n", s);

    SerializeUserInfo();

    exit(1);
}

void Logic(const Ins &i,db_api::Connector& conn) {
  switch (i.opcode) {
  case Ins::Opcode::STATISTICS: {
    replyForCommand=SendNumberOfAnswers(AllAnswers,RightAnswers, conn);         
  } break;
  case Ins::Opcode::SETSTATE:{
    replyForCommand.clear();
    SetStateToUsers(i.imms[0], i.imms[1]);   
    replyForCommand << "user states changed\n";
  } break;
  case Ins::Opcode::BAN: {
    std::cout << "banned debil number " << i.imms[0] << "\n";
  } break;
  case Ins::Opcode::PROMOTE: {
    std::cout << "promoted debil number " << i.imms[0] << "\n";
  } break;
  case Ins::Opcode::ADD_POINTS: {
    std::cout << "added " << i.imms[0] << " points to debil number "
              << i.imms[1] << "\n";
  } break;
  case Ins::Opcode::SUB_POINTS: {
    std::cout << "taken " << i.imms[0] << " points from debil number "
              << i.imms[1] << "\n";
  } break;
  case Ins::Opcode::KEK: {
    std::cout << "kek " << i.imms[0] << " " << i.imms[1] << " " << i.imms[2]
              << "\n";
  } break;
  case Ins::Opcode::INVALID:
  default: {
    std::cout << "invalid\n";
  } break;
  }
}