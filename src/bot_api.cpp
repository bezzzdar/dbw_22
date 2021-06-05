#include "bot_api.h"

namespace bot_api {

int DBot::RegisterUser(int chat_id) {
    bot_.getApi().sendMessage(
        chat_id, "Hi! let's register you. please submit your name, for example \"Vasya Vasin\"");

    bot_.getEvents().onNonCommandMessage(
        [](TgBot::Message::Ptr message) { std::cout << message->text << " registered\n"; });
}

}; // namespace bot_api