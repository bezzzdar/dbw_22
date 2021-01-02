#include <tgbot/tgbot.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <sstream>
#include <vector>

#include "bot_utils.h"
#include "db_api.h"

const char* BOT_TOKEN = "1417068350:AAGHSRRvimiHNWIMgboNm1xUr99D_7-X8gE";

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

/**
 * @brief initializes stack of tasks. tasks are read from database
 * @param stack - user's stack of tasks
 * @param conn - initialized connector to the database
 */
void InitTasksStack(TasksStack* stack, db_api::Connector& conn);

int main(int argc, char* argv[]) {
    std::string path_to_pics{argv[0]};

    const auto iter_dir = path_to_pics.rfind('/');
    path_to_pics = path_to_pics.substr(0, (iter_dir == std::string::npos) ? (0) : (iter_dir + 1));
    path_to_pics += "../../db/";

    std::cout << "path: <" << path_to_pics << ">\n";

    assert(argc == 4);

    // parameters of the DB
    const std::string hostname(argv[1]);
    const std::string username(argv[2]);
    const std::string password(argv[3]);

    // const std::string username = "root";
    // const std::string hostname = "tcp://LAPTOP-E950M0TH:3306";
    // const std::string password = "****";

    db_api::Connector conn(hostname.c_str(), username.c_str(), password.c_str(), "dialogue2020");

    TgBot::Bot bot(BOT_TOKEN);

    std::map<int, UserInfo> chat_id_to_user_info{};

    // telegram keyboards
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

    TgBot::InlineKeyboardButton::Ptr next(new TgBot::InlineKeyboardButton);
    next->text = "другой вопрос";
    next->callbackData = "next";

    TgBot::InlineKeyboardButton::Ptr choose(new TgBot::InlineKeyboardButton);
    choose->text = "выбрать тему";
    choose->callbackData = "choose";

    tasks_row0.push_back(next);
    tasks_row0.push_back(choose);

    tasks_keyboard->inlineKeyboard.push_back(tasks_row0);

    // bot startup
    bot.getEvents().onCommand("start", [&bot, &chat_id_to_user_info](TgBot::Message::Ptr message) {
        const auto chat_id = message->chat->id;

        std::stringstream reply;

        if (chat_id_to_user_info[chat_id].state < BotState::REGISTERING_NAME) {
            reply << "Привет! Скажи имя, под которым ты хочешь, чтобы я тебя "
                     "зарегистрировал\nНапример, Вася Васечкин";

            bot.getApi().sendMessage(chat_id, reply.str());

            chat_id_to_user_info[chat_id].state = BotState::REGISTERING_NAME;
        } else {
            reply << "Насколько я помню, мы уже знакомы. Нет смысла начинать все сначала :)\n";

            bot.getApi().sendMessage(chat_id, reply.str());
        }
    });

    // bot commands
    bot.getEvents().onCommand(
        "exit", [&bot, &chat_id_to_user_info, &conn](TgBot::Message::Ptr message) {
            const auto chat_id = message->chat->id;

            std::stringstream reply;

            reply << "Чтож, пока! За свои старания ты получил "
                  << conn.RequestUserScore(chat_id_to_user_info[chat_id].user_id)
                  << " условных очков. Этот результат никуда не денется, и будет сохранен в нашей "
                     "базе данных под твоим именем, не волнуйся\n";

            chat_id_to_user_info.erase(chat_id);

            bot.getApi().sendMessage(chat_id, reply.str());
        });

    // bot logic on user button input
    bot.getEvents().onCallbackQuery(
        [&bot, &tasks_keyboard, &disciplines_keyboard, &chat_id_to_user_info, &conn, &path_to_pics](
            TgBot::CallbackQuery::Ptr query) {
            // TODO: add code generation / define to avoid this copy-paste
            std::string query_data = query->data;
            const auto  chat_id = query->message->chat->id;

            auto       user_info = chat_id_to_user_info[chat_id];
            const bool can_choose_discipline = user_info.state >= BotState::NO_DISCIPLINE_CHOSEN;
            const bool is_answering_question = user_info.state > BotState::NO_DISCIPLINE_CHOSEN;

            std::stringstream reply;

            if (can_choose_discipline) {
                db_api::Disciplines discipline = db_api::Disciplines::NONE;

                if (StringTools::startsWith(query_data, "phy")) {
                    chat_id_to_user_info[chat_id].state = BotState::PHY_CHOSEN;

                    reply << "Раздел физика:\n";

                    discipline = db_api::Disciplines::PHY;
                } else if (StringTools::startsWith(query_data, "bio")) {
                    chat_id_to_user_info[chat_id].state = BotState::BIO_CHOSEN;

                    reply << "Раздел биология:\n";

                    discipline = db_api::Disciplines::BIO;
                } else if (StringTools::startsWith(query_data, "rus")) {
                    chat_id_to_user_info[chat_id].state = BotState::RUS_CHOSEN;

                    reply << "Раздел русский:\n";

                    discipline = db_api::Disciplines::RUS;
                } else if (StringTools::startsWith(query_data, "cod")) {
                    chat_id_to_user_info[chat_id].state = BotState::COD_CHOSEN;

                    reply << "Раздел кодинг:\n";

                    discipline = db_api::Disciplines::COD;
                } else if (StringTools::startsWith(query_data, "hist")) {
                    chat_id_to_user_info[chat_id].state = BotState::HIST_CHOSEN;

                    reply << "Раздел история:\n";

                    discipline = db_api::Disciplines::HIST;
                } else if (StringTools::startsWith(query_data, "chem")) {
                    chat_id_to_user_info[chat_id].state = BotState::CHEM_CHOSEN;

                    reply << "Раздел химия:\n";

                    discipline = db_api::Disciplines::CHEM;
                } else if (StringTools::startsWith(query_data, "cult")) {
                    chat_id_to_user_info[chat_id].state = BotState::CULT_CHOSEN;

                    reply << "Раздел культуры:\n";

                    discipline = db_api::Disciplines::CULT;
                } else if (StringTools::startsWith(query_data, "math")) {
                    chat_id_to_user_info[chat_id].state = BotState::MATH_CHOSEN;

                    reply << "Раздел математика:\n";

                    discipline = db_api::Disciplines::MATH;
                } else if (StringTools::startsWith(query_data, "eng")) {
                    chat_id_to_user_info[chat_id].state = BotState::ENG_CHOSEN;

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
                                                      "/" + task.pic_name;

                            std::cout << "sending photo: <" << path_to_pic << ">\n";

                            std::string mime_type = "image/";

                            const auto iter_dir = path_to_pics.rfind('.');
                            mime_type += path_to_pics.substr(iter_dir + 1);

                            bot.getApi().sendPhoto(
                                chat_id, TgBot::InputFile::fromFile(path_to_pic, mime_type));
                        }
                    } else {
                        chat_id_to_user_info[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                        reply << "К сожалению, больше вопросов в этой категории нет. Как то так. "
                                 "Выбери другую";

                        bot.getApi().sendMessage(
                            chat_id, reply.str(), false, 0, disciplines_keyboard);
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

                if (StringTools::startsWith(query_data, "next")) {
                    if (user_info.tasks_stack[discipline].empty()) {
                        return;
                    }

                    reply << "Ок. Вот следующий вопрос\n";

                    chat_id_to_user_info[chat_id].tasks_stack[discipline].push_back(
                        user_info.tasks_stack[discipline].front());

                    chat_id_to_user_info[chat_id].tasks_stack[discipline].pop_front();

                    const auto task = conn.RequestTask(
                        discipline, chat_id_to_user_info[chat_id].tasks_stack[discipline].front());

                    reply << task.text << '\n';

                    std::cout << "sending text: <" << task.text << ">\n";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);

                    if (!task.pic_name.empty()) {
                        std::string path_to_pic = path_to_pics +
                                                  db_api::discipline_to_string.at(discipline) +
                                                  "/" + task.pic_name;

                        std::cout << "sending photo: <" << path_to_pic << ">\n";

                        std::string mime_type = "image/";

                        const auto iter_dir = path_to_pics.rfind('.');
                        mime_type += path_to_pics.substr(iter_dir + 1);

                        bot.getApi().sendPhoto(chat_id,
                                               TgBot::InputFile::fromFile(path_to_pic, mime_type));
                    }
                } else if (StringTools::startsWith(query_data, "choose")) {
                    reply << "Хорошо, выбери другую тему:\n";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);

                    chat_id_to_user_info[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;
                }
            }

            return;
        });

    // bot logic on user text input
    bot.getEvents().onNonCommandMessage([&bot,
                                         &conn,
                                         &chat_id_to_user_info,
                                         &disciplines_keyboard,
                                         &tasks_keyboard,
                                         &path_to_pics](const TgBot::Message::Ptr& message) {
        const auto chat_id = message->chat->id;
        auto       message_text = message->text;

        auto       user_info = chat_id_to_user_info[chat_id];
        const auto user_id = user_info.user_id;

        std::cout << "user <" << user_info.name << "> in chat " << chat_id << " wrote:\n<"
                  << message_text << ">\n";

        db_api::Disciplines discipline = db_api::Disciplines::NONE;

        std::stringstream reply;

        // large switch-case for each user's state
        // TODO: if user state is to be remade with it's own state machine, this swould be the part
        // of the machine, something llike "enumerate_state" and each state change should be done
        // via this stete machine
        switch (chat_id_to_user_info[chat_id].state) {
        // uninitialized user
        case NO_STATE:
            bot.getApi().sendMessage(chat_id, "Чтобы начать, пожалуйста, введи команду /start");
            break;
        case REGISTERING_NAME:
            bool is_duplicate;
            is_duplicate = conn.UsernameTaken(message_text);

            if (is_duplicate) {
                reply << "Так вышло, что такого человека уже зарегистрировали. Попробуй "
                         "добавить отчество, или обратись к организаторам";
            } else if (bot_utils::IsValidName(message_text)) {
                reply << "Привет, " << message_text
                      << "\nТеперь введи номер своей школы. Только цифру, пожалуйста."
                         "Если в названии не только цифра, организаторы присвоили этой школе "
                         "какой-то номер, спроси у них, какой";

                chat_id_to_user_info[chat_id].name = bot_utils::ToLowerNoSpaces(message_text);

                chat_id_to_user_info[chat_id].state = BotState::REGISTERING_SCHOOL;
            } else {
                reply << "...\nА теперь без шуток, пожалуйста";
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
                    reply << "Уверен, школы с таким номером нет, шутник";
                }
            } catch (const std::invalid_argument& inv_arg) {
                reply << "Пожалуйста, введи только номер школы. Только цифры";

                is_valid_n = false;
            } catch (const std::out_of_range& oor) {
                reply << "Столько школ во всем мире не наберется";

                is_valid_n = false;
            }

            if (is_valid_n) {
                reply << "Здорово, ты успешно зарегистрирован как ученик школы № " << school_n
                      << "\n";

                chat_id_to_user_info[chat_id].school = school_n;
                chat_id_to_user_info[chat_id].user_id = conn.AddUser(user_info.name, school_n);
                chat_id_to_user_info[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                InitTasksStack(&chat_id_to_user_info[chat_id].tasks_stack, conn);

                reply << "Теперь выбери, какие вопросы ты хочешь решать. Категорию можно "
                         "изменить в любой момент, так что не бойся экспериментировать\n"
                         "Если ты наигрался, можешь покинуть игру при помощи команды /exit, но это "
                         "действие нельзя отменить, больше вернуться к решению вопросов после "
                         "этого ты не сможешь!\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
            } else {
                bot.getApi().sendMessage(chat_id, reply.str());
            }

            break;
        case NO_DISCIPLINE_CHOSEN:
            reply << "Жмякни на кнопку с интересующей тебя категорией, пожалуйста";

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
                const auto task_id = chat_id_to_user_info[chat_id].tasks_stack[discipline].front();
                chat_id_to_user_info[chat_id].tasks_stack[discipline].pop_front();

                conn.RegisterCorrectAnswer(user_id, discipline, task_id);

                reply << "Ответ правильный, молодец!\n";

                if (!chat_id_to_user_info[chat_id].tasks_stack[discipline].empty()) {
                    reply << "Вот следующее задание:\n";

                    const auto task = conn.RequestTask(
                        discipline, chat_id_to_user_info[chat_id].tasks_stack[discipline].front());

                    reply << task.text << '\n';

                    std::cout << "sending text: <" << task.text << ">\n";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);

                    if (!task.pic_name.empty()) {
                        std::string path_to_pic = path_to_pics +
                                                  db_api::discipline_to_string.at(discipline) +
                                                  "/" + task.pic_name;

                        std::cout << "sending photo: <" << path_to_pic << ">\n";

                        std::string mime_type = "image/";

                        const auto iter_dir = path_to_pics.rfind('.');
                        mime_type += path_to_pics.substr(iter_dir + 1);

                        bot.getApi().sendPhoto(chat_id,
                                               TgBot::InputFile::fromFile(path_to_pic, mime_type));
                    }
                } else {
                    reply << "Больше вопросов в этой категории нет. Но ты всегда можешь "
                             "попробовать другие!";

                    chat_id_to_user_info[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
                }

            } else {
                reply << "Ответ неправильный. Попытайся еще раз\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);
            }
        }

        return;
    });

    // long polling - infinite send-recieve to telegramm servers
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }

    return 1;
}

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
