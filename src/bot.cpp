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

const char* USER_LOCAL = "root";
const char* HOSTNAME_LOCAL = "tcp://LAPTOP-E950M0TH:3306";
const char* PWD_LOCAL = "vov19411945_qW";
const char* BOT_TOKEN = "1417068350:AAGHSRRvimiHNWIMgboNm1xUr99D_7-X8gE";

enum BotState {
    NO_STATE,                // current user is unknown
    REGISTERING_NAME,        // waiting for name input
    REGISTERING_NAME_CONF,   // waiting for name confirmation
    REGISTERING_SCHOOL,      // waiting for school number
    REGISTERING_SCHOOL_CONF, // waiting for school number confirmation
    NO_DISCIPLINE_CHOSEN,    // waiting for discipline choise
    PHY_CHOSEN,              // waiting for answer
    BIO_CHOSEN,              // waiting for answer
    RUS_CHOSEN,              // waiting for answer
    COD_CHOSEN,              // waiting for answer
    HIST_CHOSEN,             // waiting for answer
    CHEM_CHOSEN,             // waiting for answer
    GEN_CHOSEN,              // waiting for answer
    SOC_CHOSEN,              // waiting for answer
    MATH_CHOSEN,             // waiting for answer
};

struct UserInfo {
    std::string name = "";
    int         school = -1;

    BotState state = NO_STATE;

    int user_id = -1;

    std::list<db_api::Disciplines> depleted_disciplines{};
};

int main() {
    db_api::Connector conn(HOSTNAME_LOCAL, USER_LOCAL, PWD_LOCAL, "dialogue2020");

    TgBot::Bot bot(BOT_TOKEN);

    // BotState state_flag = BotState::NO_STATE;

    std::map<int, UserInfo> chat_id_to_user_info{};

    // keyboards
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

    TgBot::InlineKeyboardButton::Ptr gen(new TgBot::InlineKeyboardButton);
    gen->text = "общие";
    gen->callbackData = "gen";

    TgBot::InlineKeyboardButton::Ptr soc(new TgBot::InlineKeyboardButton);
    soc->text = "обществознание";
    soc->callbackData = "soc";

    TgBot::InlineKeyboardButton::Ptr math(new TgBot::InlineKeyboardButton);
    math->text = "математика";
    math->callbackData = "math";

    disciplines_row0.push_back(phy);
    disciplines_row0.push_back(bio);
    disciplines_row1.push_back(rus);
    disciplines_row1.push_back(cod);
    disciplines_row2.push_back(hist);
    disciplines_row2.push_back(chem);
    disciplines_row3.push_back(gen);
    disciplines_row3.push_back(soc);
    disciplines_row4.push_back(math);

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

    // startup
    bot.getEvents().onCommand("start", [&bot, &chat_id_to_user_info](TgBot::Message::Ptr message) {
        const auto chat_id = message->chat->id;

        std::stringstream reply;

        if (chat_id_to_user_info[chat_id].state < BotState::REGISTERING_NAME_CONF) {
            reply << "Привет! Скажи имя, под которым ты хочешь, чтобы я тебя "
                     "зарегистрировал\nНапример, Вася Васечкин";

            bot.getApi().sendMessage(chat_id, reply.str());

            chat_id_to_user_info[chat_id].state = BotState::REGISTERING_NAME;
        } else {
            reply << "Насколько я помню, мы уже знакомы. Нет смысла начинать все сначала :)\n";

            bot.getApi().sendMessage(chat_id, reply.str());
        }
    });

    // commands
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

    // unknown commands

    // buttons
    bot.getEvents().onCallbackQuery([&bot,
                                     &tasks_keyboard,
                                     &disciplines_keyboard,
                                     &chat_id_to_user_info,
                                     &conn](TgBot::CallbackQuery::Ptr query) {
        // TODO: add code generation / define to avoid this copy-paste
        std::string query_data = query->data;
        const auto  chat_id = query->message->chat->id;

        const auto user_info = chat_id_to_user_info[chat_id];
        bool       can_choose_discipline = user_info.state >= BotState::NO_DISCIPLINE_CHOSEN;
        bool       is_answering_question = user_info.state > BotState::NO_DISCIPLINE_CHOSEN;

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
            } else if (StringTools::startsWith(query_data, "gen")) {
                chat_id_to_user_info[chat_id].state = BotState::GEN_CHOSEN;

                reply << "Раздел общие вопросы:\n";

                discipline = db_api::Disciplines::GEN;
            } else if (StringTools::startsWith(query_data, "soc")) {
                chat_id_to_user_info[chat_id].state = BotState::SOC_CHOSEN;

                reply << "Раздел обществознание:\n";

                discipline = db_api::Disciplines::SOC;
            } else if (StringTools::startsWith(query_data, "math")) {
                chat_id_to_user_info[chat_id].state = BotState::MATH_CHOSEN;

                reply << "Раздел математика:\n";

                discipline = db_api::Disciplines::MATH;
            }

            if (discipline != db_api::Disciplines::NONE) {
                // FIXME: no support for image questions
                bool is_depleted = std::find(user_info.depleted_disciplines.begin(),
                                             user_info.depleted_disciplines.end(),
                                             discipline) != user_info.depleted_disciplines.end();

                if (!is_depleted) {
                    const auto task = conn.RequestUserTask(user_info.user_id, discipline);

                    reply << task << '\n';

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);
                } else {
                    reply << "К сожалению, больше вопросов в этой категории нет. Как то так. "
                             "Выбери другую";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
                }
            }
        }

        if (is_answering_question) {
            if (StringTools::startsWith(query_data, "next")) {

                reply << "<Функция под вопросом, пока что не работает>\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);
            } else if (StringTools::startsWith(query_data, "choose")) {
                reply << "Хорошо, выбери другую тему:\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);

                chat_id_to_user_info[chat_id].state = BotState::NO_DISCIPLINE_CHOSEN;
            }
        }

        return;
    });

    // main logic
    bot.getEvents().onNonCommandMessage([&bot,
                                         &conn,
                                         &chat_id_to_user_info,
                                         &disciplines_keyboard,
                                         &tasks_keyboard](const TgBot::Message::Ptr& message) {
        const auto chat_id = message->chat->id;
        auto       message_text = message->text;

        std::cout << "user in chat " << chat_id << " wrote:\n<" << message_text << ">\n";

        db_api::Disciplines current_discipline = db_api::Disciplines::NONE;

        std::stringstream reply;

        switch (chat_id_to_user_info[chat_id].state) {
        case NO_STATE:
            bot.getApi().sendMessage(chat_id, "Чтобы начать, пожалуйста, введи команду /start");
            break;
        case REGISTERING_NAME:
            reply << "Я распознал твое имя как\n\'" << message_text << "\'\n";

            bool is_duplicate;
            is_duplicate = conn.UsernameTaken(message_text);

            if (is_duplicate) {
                reply << "Так вышло, что такого человека уже зарегистрировали. Попробуй "
                         "добавить отчество, или обратись к организаторам";
            } else if (bot_utils::IsValidName(message_text)) {
                reply << "Уверен, что хочешь оставить его таким?\nДа/Нет";

                chat_id_to_user_info[chat_id].name = bot_utils::ToLowerNoSpaces(message_text);

                chat_id_to_user_info[chat_id].state = BotState::REGISTERING_NAME_CONF;
            } else {
                reply << "...\nА теперь без шуток, пожалуйста";
            };

            bot.getApi().sendMessage(chat_id, reply.str());

            break;
        case REGISTERING_NAME_CONF:
            message_text = bot_utils::ToLowerNoSpaces(message_text);

            if (message_text == "да") {
                reply << "Здорово, теперь введи номер своей школы. Только цифру, пожалуйста."
                         "Если в названии не только цифра, организаторы присвоили этой школе "
                         "какой-то номер, спроси у них, какой ";
                chat_id_to_user_info[chat_id].state = REGISTERING_SCHOOL;
            } else if (message_text == "нет") {
                reply << "Здорово, тогда введи свое имя заново, пожалуйста";
                chat_id_to_user_info[chat_id].state = BotState::REGISTERING_NAME;
            } else {
                reply << "Что то странное, я не понял, что ты написал. Введи, пожалуйста, "
                         "ответ еще раз";
            }

            bot.getApi().sendMessage(chat_id, reply.str());

            break;
        case REGISTERING_SCHOOL:
            int  school_n;
            bool is_valid_n;

            try {
                school_n = std::stoi(message_text);

                if (school_n < 0) {
                    reply << "Уверен, школы с таким номером нет";

                    is_valid_n = false;
                } else {
                    is_valid_n = true;
                }
            } catch (const std::invalid_argument& inv_arg) {
                reply << "Пожалуйста, введи только номер школы. Только цифры";

                is_valid_n = false;
            } catch (const std::out_of_range& oor) {
                reply << "Уверен, школы с таким номером нет";

                is_valid_n = false;
            }

            if (is_valid_n) {
                reply << "Я распознал твою школу  как \'";
                switch (school_n) {
                // FIXME: add specific schools like liceum, etc
                default:
                    reply << school_n;
                    break;
                }
                reply << "\'\nТы уверен, что это твоя школа?\nДа/Нет";

                chat_id_to_user_info[chat_id].school = school_n;
                chat_id_to_user_info[chat_id].state = BotState::REGISTERING_SCHOOL_CONF;
            }

            bot.getApi().sendMessage(chat_id, reply.str());

            break;
        case REGISTERING_SCHOOL_CONF:
            message_text = bot_utils::ToLowerNoSpaces(message_text);

            if (message_text == "да") {
                const auto info = chat_id_to_user_info[chat_id];

                chat_id_to_user_info[chat_id].user_id = conn.AddUser(info.name, info.school);

                reply << "Здорово, ты зарегистрирован под именем " << info.name << " из школы № "
                      << info.school << "\nтвой id: " << chat_id_to_user_info[chat_id].user_id
                      << "\n";

                chat_id_to_user_info[chat_id].state = NO_DISCIPLINE_CHOSEN;

                reply << "Теперь выбери, какие вопросы ты хочешь решать. Категорию можно "
                         "изменить в любой момент, так что не бойся экспериментировать\n"
                         "Если ты наигрался, можешь покинуть игру при помощи команды /exit, но это "
                         "действие нельзя отменить, больше вернуться к решению вопросов после "
                         "этого ты не сможешь!\n";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);

                return;
            } else if (message_text == "нет") {
                reply << "Здорово, тогда введи свою школу заново, пожалуйста";
                chat_id_to_user_info[chat_id].state = BotState::REGISTERING_SCHOOL;
            } else {
                reply << "Что то странное, я не понял, что ты написал. Введи, пожалуйста, "
                         "ответ еще раз";
            }

            bot.getApi().sendMessage(chat_id, reply.str());
            break;
        case NO_DISCIPLINE_CHOSEN:
            reply << "Жмякни на кнопку с интересующей тебя категорией, пожалуйста";

            bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);
            break;
        case PHY_CHOSEN:
            current_discipline = db_api::Disciplines::PHY;
            break;
        case BIO_CHOSEN:
            current_discipline = db_api::Disciplines::BIO;
            break;
        case RUS_CHOSEN:
            current_discipline = db_api::Disciplines::RUS;
            break;
        case COD_CHOSEN:
            current_discipline = db_api::Disciplines::COD;
            break;
        case HIST_CHOSEN:
            current_discipline = db_api::Disciplines::HIST;
            break;
        case CHEM_CHOSEN:
            current_discipline = db_api::Disciplines::CHEM;
            break;
        case GEN_CHOSEN:
            current_discipline = db_api::Disciplines::GEN;
            break;
        case SOC_CHOSEN:
            current_discipline = db_api::Disciplines::SOC;
            break;
        case MATH_CHOSEN:
            current_discipline = db_api::Disciplines::MATH;
            break;
        default:
            current_discipline = db_api::Disciplines::PHY;
            break;
        }

        if (current_discipline != db_api::Disciplines::NONE) {
            const auto user_info = chat_id_to_user_info[chat_id];
            const auto user_id = user_info.user_id;

            message_text = bot_utils::ToLowerNoSpaces(message_text);

            bool ans_is_correct = conn.CheckUserAnswer(user_id, current_discipline, message_text);
            if (ans_is_correct) {
                bool no_more_questions = false;
                conn.RegisterCorrectAnswer(user_id, current_discipline, &no_more_questions);

                bool in_depleted_list =
                    std::find(user_info.depleted_disciplines.begin(),
                              user_info.depleted_disciplines.end(),
                              current_discipline) != user_info.depleted_disciplines.end();

                bool is_depleted = no_more_questions || in_depleted_list;

                if (no_more_questions) {
                    chat_id_to_user_info[chat_id].depleted_disciplines.push_back(
                        current_discipline);
                }

                reply << "Ответ правильный, молодец!\n";

                if (!is_depleted) {
                    reply << "Вот следующее задание:\n";
                    reply << conn.RequestUserTask(user_id, current_discipline);

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);
                } else {
                    reply << "Больше вопросов в этой категории нет. Но ты всегда можешь "
                             "попробовать другие!";

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