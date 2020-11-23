#include <tgbot/tgbot.h>

#include <algorithm>
#include <cctype>
#include <iostream>
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

struct UserInfo {
    std::string name;
    int school;
    int user_id;
};

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

int main() {
    db_api::Connector conn(HOSTNAME_LOCAL, USER_LOCAL, PWD_LOCAL, "dialogue2020");

    TgBot::Bot bot(BOT_TOKEN);

    BotState state_flag = BotState::NO_STATE;

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

    TgBot::InlineKeyboardMarkup::Ptr tasks_keyboard(new TgBot::InlineKeyboardMarkup);
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
    bot.getEvents().onCommand("start", [&bot, &state_flag](TgBot::Message::Ptr message) {
        if (state_flag < BotState::REGISTERING_NAME_CONF) {
            bot.getApi().sendMessage(message->chat->id,
                                     "Привет! Скажи имя, под которым ты хочешь, чтобы я тебя"
                                     " зарегистрировал\nНапример, Вася Васечкин");
            state_flag = BotState::REGISTERING_NAME;
        } else {
            bot.getApi().sendMessage(
                message->chat->id,
                "Насколько я помню, мы уже знакомы. Нет смысла начинать все сначала :)\n");
        }
    });

    // commands

    // unknown commands

    // buttons

    bot.getEvents().onCallbackQuery([&bot, &tasks_keyboard, &disciplines_keyboard, &state_flag](
                                        TgBot::CallbackQuery::Ptr query) {
        // TODO: add code generation / define to avoid this copy-paste
        std::string query_data = query->data;
        const auto chat_id = query->message->chat->id;

        bool can_choose_discipline = state_flag >= BotState::NO_DISCIPLINE_CHOSEN;
        bool is_answering_question = state_flag > BotState::NO_DISCIPLINE_CHOSEN;

        std::stringstream reply;
        db_api::Disciplines discipline = db_api::Disciplines::NONE;

        if (can_choose_discipline) {
            if (StringTools::startsWith(query_data, "phy")) {
                state_flag = BotState::PHY_CHOSEN;

                reply << "Раздел физика:";

                discipline = db_api::Disciplines::PHY;
            } else if (StringTools::startsWith(query_data, "bio")) {
                state_flag = BotState::BIO_CHOSEN;

                reply << "Раздел биология:";

                discipline = db_api::Disciplines::BIO;
            } else if (StringTools::startsWith(query_data, "rus")) {
                state_flag = BotState::RUS_CHOSEN;

                reply << "Раздел русский:";

                discipline = db_api::Disciplines::RUS;
            } else if (StringTools::startsWith(query_data, "cod")) {
                state_flag = BotState::COD_CHOSEN;

                reply << "Раздел кодинг:";

                discipline = db_api::Disciplines::COD;
            } else if (StringTools::startsWith(query_data, "hist")) {
                state_flag = BotState::HIST_CHOSEN;

                reply << "Раздел история:";

                discipline = db_api::Disciplines::HIST;
            } else if (StringTools::startsWith(query_data, "chem")) {
                state_flag = BotState::CHEM_CHOSEN;

                reply << "Раздел химия:";

                discipline = db_api::Disciplines::CHEM;
            } else if (StringTools::startsWith(query_data, "gen")) {
                state_flag = BotState::GEN_CHOSEN;

                reply << "Раздел общие вопросы:";

                discipline = db_api::Disciplines::GEN;
            } else if (StringTools::startsWith(query_data, "soc")) {
                state_flag = BotState::SOC_CHOSEN;

                reply << "Раздел обществознание:";

                discipline = db_api::Disciplines::SOC;
            } else if (StringTools::startsWith(query_data, "math")) {
                state_flag = BotState::MATH_CHOSEN;

                reply << "Раздел математика:";

                discipline = db_api::Disciplines::MATH;
            }

            // TODO: add actual question query
            if (discipline != db_api::Disciplines::NONE) {
                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);
            }
        }

        if (is_answering_question) {
            if (StringTools::startsWith(query_data, "next")) {

                reply << "Хорошо, вот другой вопрос:";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, tasks_keyboard);
            } else if (StringTools::startsWith(query_data, "choose")) {
                reply << "Хорошо, выбери другую тему:";

                bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);

                state_flag = BotState::NO_DISCIPLINE_CHOSEN;
            }
        }

        return;
    });

    // main logic
    bot.getEvents().onNonCommandMessage(
        [&bot, &state_flag, &conn, &chat_id_to_user_info, &disciplines_keyboard, &tasks_keyboard](
            const TgBot::Message::Ptr& message) {
            const auto chat_id = message->chat->id;
            auto message_text = message->text;
            std::cout << "user in chat " << chat_id << " wrote:\n<" << message_text << ">\n";

            std::stringstream reply;

            switch (state_flag) {
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

                    state_flag = BotState::REGISTERING_NAME_CONF;
                } else {
                    reply << "...\nА теперь без шуток, пожалуйста";
                };

                bot.getApi().sendMessage(chat_id, reply.str());

                break;
            case REGISTERING_NAME_CONF:
                message_text = bot_utils::ToLowerNoSpaces(message_text);

                if (message_text == "да") {
                    reply << "Здорово, теперь введи номер своей школы. Только цифру, пожалуйста."
                             "Если в названии не только цифра, организаторы присвоили этой школе"
                             "какой-то номер, спроси у них, какой ";
                    state_flag = REGISTERING_SCHOOL;
                } else if (message_text == "нет") {
                    reply << "Здорово, тогда введи свое имя заново, пожалуйста";
                    state_flag = BotState::REGISTERING_NAME;
                } else {
                    reply << "Что то странное, я не понял, что ты написал. Введи, пожалуйста, "
                             "ответ еще раз";
                }

                bot.getApi().sendMessage(chat_id, reply.str());

                break;
            case REGISTERING_SCHOOL:
                int school_n;
                bool is_valid_n;

                try {
                    school_n = std::stoi(message_text);

                    is_valid_n = true;
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

                    state_flag = BotState::REGISTERING_SCHOOL_CONF;
                }

                bot.getApi().sendMessage(chat_id, reply.str());

                break;
            case REGISTERING_SCHOOL_CONF:
                message_text = bot_utils::ToLowerNoSpaces(message_text);

                if (message_text == "да") {
                    const auto info = chat_id_to_user_info[chat_id];

                    chat_id_to_user_info[chat_id].user_id = conn.AddUser(info.name, info.school);

                    reply << "Здорово, ты зарегистрирован под именем " << info.name
                          << " из школы № " << info.school
                          << "\nтвой id: " << chat_id_to_user_info[chat_id].user_id << "\n";

                    state_flag = NO_DISCIPLINE_CHOSEN;

                    reply << "Теперь выбери, какие вопросы ты хочешь решать. Категорию можно "
                             "изменить в любой момент, так что не бойся экспериментировать\n";

                    bot.getApi().sendMessage(chat_id, reply.str(), false, 0, disciplines_keyboard);

                    // FIXME: add transition to the disciplines section
                    return;
                } else if (message_text == "нет") {
                    reply << "Здорово, тогда введи свою школу заново, пожалуйста";
                    state_flag = BotState::REGISTERING_SCHOOL;
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
                // TODO:
                break;
            case BIO_CHOSEN:
                // TODO:
                break;
            case RUS_CHOSEN:
                // TODO:
                break;
            case COD_CHOSEN:
                // TODO:
                break;
            case HIST_CHOSEN:
                // TODO:
                break;
            case CHEM_CHOSEN:
                // TODO:
                break;
            case GEN_CHOSEN:
                // TODO:
                break;
            case SOC_CHOSEN:
                // TODO:
                break;
            case MATH_CHOSEN:
                // TODO:
                break;
            default:
                // TODO:
                break;
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