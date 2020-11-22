#ifndef __DIALOGUE_BOT_API_H_INCLUDED__
#define __DIALOGUE_BOT_API_H_INCLUDED__

#include <tgbot/tgbot.h>

#include <memory>

#include "db_api.h"

namespace bot_api {
struct State {
    // int user_id_bd;

    std::map<int, int> chat_id_to_db_user_id;
};

class DBot {
  public:
    DBot() = delete;
    DBot(const char* bot_id, std::unique_ptr<db_api::Connector> db_connector) : bot_(bot_id) {
        db_connector_ = std::move(db_connector);
    };

    void Help(int chat_id);
    int  RegisterUser(int chat_id);
    int  RequestNewTask(int chat_id);
    int  ChangeDiscipline(int chat_id);

  private:
    TgBot::Bot                         bot_;
    std::unique_ptr<db_api::Connector> db_connector_;

    State state_;
};
}; // namespace bot_api

#endif