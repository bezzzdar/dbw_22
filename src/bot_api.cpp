#include "bot_api.h"

namespace bot_api {

// int DBot::Help() {
//     bot_.getApi().se
// }

int DBot::RegisterUser(int chat_id) {
    bot_.getApi().sendMessage(
        chat_id, "Hi! let's register you. please submit your name, for example \"Vasya Vasin\"");

    bot_.getEvents().onNonCommandMessage([](TgBot::Message::Ptr message) {
        std::cout << message->text << " registered\n";
    });
}

// int DBot::RequestNewTask();

// int DBot::ChangeDiscipline();

}; // namespace bot_api