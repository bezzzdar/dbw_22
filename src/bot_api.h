#ifndef __DIALOGUE_BOT_API_H_INCLUDED__
#define __DIALOGUE_BOT_API_H_INCLUDED__

#include <tgbot/tgbot.h>

#include <memory>

#include "db_api.h"

namespace bot_api {
struct State {
    // bot's state. chat id in telegram to id in database
    std::map<int, int> chat_id_to_db_user_id;
};

class DBot {
  public:
    DBot() = delete;
    DBot(const char* bot_id, std::unique_ptr<db_api::Connector> db_connector) : bot_(bot_id) {
        db_connector_ = std::move(db_connector);
    };

    void Help(int chat_id); // unimplemented, done inside bot's switch-case

    int RegisterUser(int chat_id); // unused, done inside bot's switch-case

    int RequestNewTask(int chat_id); // unimplemented, done inside bot's switch-case

    int ChangeDiscipline(int chat_id); // unimplemented, done inside bot's switch-case

  private:
    TgBot::Bot                         bot_;
    std::unique_ptr<db_api::Connector> db_connector_;

    State state_;
};
}; // namespace bot_api

#endif