#include <tgbot/tgbot.h>

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

const char* BOT_TOKEN = "1417068350:AAGHSRRvimiHNWIMgboNm1xUr99D_7-X8gE";

// ====================
// TYPES

// user's possible states.
// NOTE: needs it's own state machine to avoid illegal states and allow for clearer state changes
enum BotState {
    NO_STATE,             // current user is unknown
    REGISTERING_NAME,     // waiting for name input
    REGISTERING_SCHOOL,   // waiting for school number
    NO_DISCIPLINE_CHOSEN, // waiting for discipline choise
    PHY_CHOSEN,           // waiting for answer
    BIO_CHOSEN,           // waiting for answer
    RUS_CHOSEN,           // waiting for answer
    COD_CHOSEN,           // waiting for answer
    HIST_CHOSEN,          // waiting for answer
    CHEM_CHOSEN,          // waiting for answer
    ENG_CHOSEN,           // waiting for answer
    CULT_CHOSEN,          // waiting for answer
    MATH_CHOSEN,          // waiting for answer
};

typedef std::map<db_api::Disciplines, std::list<size_t>> TasksStack;

// information about each user. can be broadened if needed
struct UserInfo {
    std::string name = "";
    int         school = -1;

    BotState state = NO_STATE;

    int user_id = -1;

    TasksStack tasks_stack;
};

// ====================
// FUNC DECLARATION
void InitTasksStack(TasksStack* stack, db_api::Connector& conn);
void ReadUserInfo(const std::string& path_to_save);
void SerializeUserInfo();
void SigHandler(int s);

// ====================
// GLOBALS
const char*             BOT_TOKEN = "1417068350:AAGHSRRvimiHNWIMgboNm1xUr99D_7-X8gE";
std::map<int, UserInfo> CHAT_ID_TO_USER_INFO{};

// ====================
// MAIN
int main(int argc, char* argv[]) {
    // back-up data storage
    // on unhandled exception
    std::set_terminate([]() {
        std::cout << "Unhandled exception or abort occured!\n" << std::endl;

        SerializeUserInfo();

        std::abort();
    });

    // on normal termination
    std::atexit([]() {
        std::cout << "Bot terminated!\n" << std::endl;

        SerializeUserInfo();
    });

    // on signal
    struct sigaction sig_action;

    sig_action.sa_handler = SigHandler;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    sigaction(SIGINT, &sig_action, NULL);

    // arg parsing
    // assert(argc == 4);

    // parameters of the DB
    const std::string hostname(argv[1]);
    const std::string username(argv[2]);
    const std::string password(argv[3]);

    // const std::string username = "root";
    // const std::string hostname = "tcp://LAPTOP-E950M0TH:3306";
    // const std::string password = "****";

    const std::string path_to_save = (argc == 5) ? (std::string(argv[4])) : (std::string(""));

    // path initialization
    std::string path_to_pics{argv[0]};

    const auto iter_dir = path_to_pics.rfind('/');
    path_to_pics = path_to_pics.substr(0, (iter_dir == std::string::npos) ? (0) : (iter_dir + 1));
    path_to_pics += "../../db/";

    std::cout << "path: <" << path_to_pics << ">\n";

    // CHAT_ID_TO_USER_INFO initialization
    if (!path_to_save.empty()) {
        std::cout << "Reading user info from " << path_to_save << "\n";

        // TODO: error handling
        ReadUserInfo(path_to_save);

        std::cout << "Successfully read user data\n";
    }

    // main entities
    db_api::Connector conn(hostname.c_str(), username.c_str(), password.c_str(), "dialogue2020");

    TgBot::Bot bot(BOT_TOKEN);

    // setting up telegram keyboards
    TgBot::InlineKeyboardMarkup::Ptr disciplines_keyboard(new TgBot::InlineKeyboardMarkup);
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row0;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row1;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row2;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row3;
    std::vector<TgBot::InlineKeyboardButton::Ptr> disciplines_row4;

    TgBot::InlineKeyboardButton::Ptr phy(new TgBot::InlineKeyboardButton);
    phy->text = "физика";
    phy->callbackData = "phy";

    TgBot::InlineKeyboardButton::Ptr bio(new TgBot::InlineKeyboardButton);
    bio->text = "биология";
    bio->callbackData = "bio";

    TgBot::InlineKeyboardButton::Ptr rus(new TgBot::InlineKeyboardButton);
    rus->text = "русский";
    rus->callbackData = "rus";

    TgBot::InlineKeyboardButton::Ptr cod(new TgBot::InlineKeyboardButton);
    cod->text = "кодинг";
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

    disciplines_row0.push_back(phy);
    disciplines_row0.push_back(bio);
    disciplines_row1.push_back(rus);
    disciplines_row1.push_back(cod);
    disciplines_row2.push_back(hist);
    disciplines_row2.push_back(chem);
    disciplines_row3.push_back(cult);
    disciplines_row3.push_back(math);
    disciplines_row4.push_back(eng);

    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row0);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row1);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row2);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row3);
    disciplines_keyboard->inlineKeyboard.push_back(disciplines_row4);

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
    bot.getEvents().onCommand(
        "start", [&bot /*, &CHAT_ID_TO_USER_INFO */](TgBot::Message::Ptr message) {
            const auto chat_id = message->chat->id;

            std::stringstream reply;

            if (CHAT_ID_TO_USER_INFO[chat_id].state < BotState::REGISTERING_NAME) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::REGISTERING_NAME;

                reply << "Привет! Скажи имя, под которым ты хочешь, чтобы я тебя "
                         "зарегистрировал в формате Имя Фамилия, пожалуйста\n";

                bot.getApi().sendMessage(chat_id, reply.str());
            } else {
                reply << "Жду ввода имени...\n";

                bot.getApi().sendMessage(chat_id, reply.str());
            }
        });

    // bot additional commands
    bot.getEvents().onCommand(
        "exit", [&bot, /* &CHAT_ID_TO_USER_INFO, */ &conn](TgBot::Message::Ptr message) {
            const auto chat_id = message->chat->id;

            std::stringstream reply;

            CHAT_ID_TO_USER_INFO.erase(chat_id);

            reply << "Чтож, пока! За свои старания ты получил(а) "
                  << conn.RequestUserScore(CHAT_ID_TO_USER_INFO[chat_id].user_id)
                  << " условных очков. Этот результат никуда не денется, и будет сохранен в нашей "
                     "базе данных под твоим именем, не волнуйся\n";

            bot.getApi().sendMessage(chat_id, reply.str());
        });

    // bot logic on user button input
    bot.getEvents().onCallbackQuery([&bot,
                                     &tasks_keyboard,
                                     &disciplines_keyboard,
                                     /* &CHAT_ID_TO_USER_INFO, */ &conn,
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
            } else if (StringTools::startsWith(query_data, "rus")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::RUS_CHOSEN;

                reply << "Раздел русский:\n";

                discipline = db_api::Disciplines::RUS;
            } else if (StringTools::startsWith(query_data, "cod")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::COD_CHOSEN;

                reply << "Раздел кодинг:\n";

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

                reply << "Раздел культуры:\n";

                discipline = db_api::Disciplines::CULT;
            } else if (StringTools::startsWith(query_data, "math")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::MATH_CHOSEN;

                reply << "Раздел математика:\n";

                discipline = db_api::Disciplines::MATH;
            } else if (StringTools::startsWith(query_data, "eng")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::ENG_CHOSEN;

                reply << "Раздел английский:\n";

                discipline = db_api::Disciplines::ENG;
            }

            if (discipline != db_api::Disciplines::NONE) {
                bool is_depleted = user_info.tasks_stack[discipline].empty();

                if (!is_depleted) {
                    const auto task =
                        conn.RequestTask(discipline, user_info.tasks_stack[discipline].front());

                    reply << task.text << '\n';

                    std::cout << "sending text: <" << task.text << ">\n";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);

                    if (!task.pic_name.empty()) {
                        std::string path_to_pic = path_to_pics +
                                                  db_api::discipline_to_string.at(discipline) +
                                                  '/' + task.pic_name + '\0';

                        std::cout << "sending photo: <" << path_to_pic << ">\n";

                        std::string mime_type = "image/";

                        const auto iter_dir = path_to_pics.rfind('.');
                        mime_type += path_to_pics.substr(iter_dir + 1);

                        bot.getApi().sendPhoto(chat_id,
                                               TgBot::InputFile::fromFile(path_to_pic, mime_type));
                    }
                } else {
                    CHAT_ID_TO_USER_INFO[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                    reply << "К сожалению, больше вопросов в этой категории нет, выбери другую, "
                             "пожалуйста\n";

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
            case BotState::RUS_CHOSEN:
                discipline = db_api::Disciplines::RUS;
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
                    discipline, CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].front());

                reply << task.text << '\n';

                std::cout << "sending text: <" << task.text << ">\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);

                if (!task.pic_name.empty()) {
                    std::string path_to_pic = path_to_pics +
                                              db_api::discipline_to_string.at(discipline) + '/' +
                                              task.pic_name + '\0';

                    std::cout << "sending photo: <" << path_to_pic << ">\n";

                    std::string mime_type = "image/";

                    const auto iter_dir = path_to_pics.rfind('.');
                    mime_type += path_to_pics.substr(iter_dir + 1);

                    bot.getApi().sendPhoto(chat_id,
                                           TgBot::InputFile::fromFile(path_to_pic, mime_type));
                }
            } else if (StringTools::startsWith(query_data, "choose_button")) {
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                reply << "Хорошо, вот темы на выбор:\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
            }
        }

        return;
    });

    // bot logic on any non-command message
    bot.getEvents().onNonCommandMessage([&bot,
                                         &conn,
                                         /* &CHAT_ID_TO_USER_INFO, */
                                         &disciplines_keyboard,
                                         &tasks_keyboard,
                                         &path_to_pics](const TgBot::Message::Ptr& message) {
        const auto chat_id = message->chat->id;
        auto       message_text = message->text;

        auto       user_info = CHAT_ID_TO_USER_INFO[chat_id];
        const auto user_id = user_info.user_id;

        std::cout << "user <" << user_info.name << "> in chat " << chat_id << " wrote:\n<"
                  << message_text << ">\n";

        db_api::Disciplines discipline = db_api::Disciplines::NONE;

        std::stringstream reply;

        // large switch-case for each user's state
        // TODO: if user state is to be remade with it's own state machine, this swould be the part
        // of the machine, something llike "enumerate_state" and each state change should be done
        // via this stete machine
        switch (CHAT_ID_TO_USER_INFO[chat_id].state) {
        case NO_STATE:
            bot.getApi().sendMessage(chat_id, "Чтобы начать, введи команду /start, пожалуйста\n");
            break;
        case REGISTERING_NAME:
            bool is_duplicate;
            is_duplicate = conn.UsernameTaken(message_text);

            if (is_duplicate) {
                reply << "Так вышло, что человека с таким именем уже зарегистрировали. Обратись к "
                         "организаторам, пожалуйста\n";
            } else if (bot_utils::IsValidName(message_text)) {
                reply << "Привет, " << message_text
                      << "\nТеперь введи номер своей школы. Только цифру, пожалуйста\n"
                         "Если в названии школы не только цифра, организаторы присвоили этой школе "
                         "какой-то номер, спроси у них, какой\n";

                CHAT_ID_TO_USER_INFO[chat_id].name = bot_utils::ToLowerNoSpaces(message_text);

                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::REGISTERING_SCHOOL;
            } else {
                reply << "...\nА теперь без шуток, пожалуйста\n";
            };

            bot.getApi().sendMessage(chat_id, reply.str());

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
                reply << "Здорово, ты успешно зарегистрирован(а) как ученик школы № " << school_n
                      << "\n";

                CHAT_ID_TO_USER_INFO[chat_id].school = school_n;
                CHAT_ID_TO_USER_INFO[chat_id].user_id = conn.AddUser(user_info.name, school_n);
                CHAT_ID_TO_USER_INFO[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                InitTasksStack(&CHAT_ID_TO_USER_INFO[chat_id].tasks_stack, conn);

                reply << "Теперь выбери, какие вопросы хочешь решать. Категорию можно "
                         "изменить в любой момент, так что не бойся экспериментировать\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
            } else {
                bot.getApi().sendMessage(chat_id, reply.str());
            }

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
        case RUS_CHOSEN:
            discipline = db_api::Disciplines::RUS;
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
        default:
            discipline = db_api::Disciplines::PHY;
            break;
        }

        // checking answer
        if (discipline != db_api::Disciplines::NONE) {
            bool ans_is_correct = conn.CheckAnswer(
                message_text, discipline, user_info.tasks_stack[discipline].front());

            if (ans_is_correct) {
                const auto task_id = CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].front();

                conn.RegisterCorrectAnswer(user_id, discipline, task_id);

                CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].pop_front();

                reply << "Ответ правильный, молодец!\n";

                if (!CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].empty()) {
                    reply << "Вот следующее задание:\n";

                    const auto task = conn.RequestTask(
                        discipline, CHAT_ID_TO_USER_INFO[chat_id].tasks_stack[discipline].front());

                    reply << task.text << '\n';

                    std::cout << "sending text: <" << task.text << ">\n";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);

                    if (!task.pic_name.empty()) {
                        std::string path_to_pic = path_to_pics +
                                                  db_api::discipline_to_string.at(discipline) +
                                                  '/' + task.pic_name + '\0';

                        std::cout << "sending photo: <" << path_to_pic << ">\n";

                        std::string mime_type = "image/";

                        const auto iter_dir = path_to_pics.rfind('.');
                        mime_type += path_to_pics.substr(iter_dir + 1);

                        bot.getApi().sendPhoto(chat_id,
                                               TgBot::InputFile::fromFile(path_to_pic, mime_type));
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

    std::cout << "Bot username: " << bot.getApi().getMe()->username.c_str() << "\n";

    TgBot::TgLongPoll longPoll(bot);

    try {
        while (true) {
            std::cout << "Long poll started\n";

            longPoll.start();
        }
    } catch (const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;

        SerializeUserInfo();
    } catch (const std::exception& ex) {
        std::cerr << "Error occurred: " << ex.what() << std::endl;

        SerializeUserInfo();
    }

    return 0;
}

// ====================
// FUNC IMPLEMENTATION

void InitDiscipline(std::list<size_t>* tasks, const size_t n_tasks) {
    for (size_t i = 1; i <= n_tasks; i++) {
        tasks->push_back(i);
    }

    return;
}

void InitTasksStack(TasksStack* stack, db_api::Connector& conn) {
    InitDiscipline(&((*stack)[db_api::Disciplines::PHY]),
                   conn.RequestNumberTasks(db_api::Disciplines::PHY));
    InitDiscipline(&((*stack)[db_api::Disciplines::MATH]),
                   conn.RequestNumberTasks(db_api::Disciplines::MATH));
    InitDiscipline(&((*stack)[db_api::Disciplines::RUS]),
                   conn.RequestNumberTasks(db_api::Disciplines::RUS));
    InitDiscipline(&((*stack)[db_api::Disciplines::BIO]),
                   conn.RequestNumberTasks(db_api::Disciplines::BIO));
    InitDiscipline(&((*stack)[db_api::Disciplines::COD]),
                   conn.RequestNumberTasks(db_api::Disciplines::COD));
    InitDiscipline(&((*stack)[db_api::Disciplines::CULT]),
                   conn.RequestNumberTasks(db_api::Disciplines::CULT));
    InitDiscipline(&((*stack)[db_api::Disciplines::HIST]),
                   conn.RequestNumberTasks(db_api::Disciplines::HIST));
    InitDiscipline(&((*stack)[db_api::Disciplines::CHEM]),
                   conn.RequestNumberTasks(db_api::Disciplines::CHEM));
    InitDiscipline(&((*stack)[db_api::Disciplines::ENG]),
                   conn.RequestNumberTasks(db_api::Disciplines::ENG));

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

        for (size_t i = 5; i < tokens.size(); i++) {
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
    std::ofstream f;
    f.open("chat_id_to_u_info.log", std::ofstream::out | std::ofstream::trunc);

    for (const auto& pair_info : CHAT_ID_TO_USER_INFO) {
        f << pair_info.first << ';';
        f << pair_info.second.name << ';' << pair_info.second.school << ';'
          << pair_info.second.state << ';' << pair_info.second.user_id;

        for (const auto& pair_stack : pair_info.second.tasks_stack) {
            f << ';' << pair_stack.first;

            for (const auto& task_id : pair_stack.second) {
                f << ',' << task_id;
            }
        }

        f << '\n';
    }

    f.close();

    return;
}

void SigHandler(int s) {
    printf("Caught signal %d\n", s);

    // SerializeUserInfo();

    exit(1);
}