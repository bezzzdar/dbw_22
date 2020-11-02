// #include <stdio.h>
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

using ::std::cout;
using ::std::endl;
using namespace ::mysqlx;

int main(int argc, const char* argv[]) try {

    // const char* url = (argc > 1 ? argv[1] : "mysqlx://");

    // cout << "Creating session on " << url << " ..." << endl;

    // Session sess(url);

    Session sess("kompuhter", 33060, "root", "../db/db.sql", "dialogue2020");

    cout << "Session accepted, creating collection..." << endl;

    Schema     sch  = sess.getSchema("test");
    Collection coll = sch.createCollection("c1", true);

    cout << "Inserting documents..." << endl;

    coll.remove("true").execute();

    {
        DbDoc doc(R"({ "name": "foo", "age": 1 })");

        Result add = coll.add(doc)
                         .add(R"({ "name": "bar", "age": 2, "toys": [ "car", "ball" ] })")
                         .add(R"({ "name": "bar", "age": 2, "toys": [ "car", "ball" ] })")
                         .add(R"({
                 "name": "baz",
                  "age": 3,
                 "date": { "day": 20, "month": "Apr" }
              })")
                         .add(R"({ "_id": "myuuid-1", "name": "foo", "age": 7 })")
                         .execute();

        std::list<string> ids = add.getGeneratedIds();
        for (string id : ids)
            cout << "- added doc with id: " << id << endl;
    }

    cout << "Fetching documents..." << endl;

    DocResult docs = coll.find("age > 1 and name like 'ba%'").execute();

    int i = 0;
    for (DbDoc doc : docs) {
        cout << "doc#" << i++ << ": " << doc << endl;

        for (Field fld : doc) {
            cout << " field `" << fld << "`: " << doc[fld] << endl;
        }

        string name = doc["name"];
        cout << " name: " << name << endl;

        if (doc.hasField("date") && Value::DOCUMENT == doc.fieldType("date")) {
            cout << "- date field" << endl;
            DbDoc date = doc["date"];
            for (Field fld : date) {
                cout << "  date `" << fld << "`: " << date[fld] << endl;
            }
            string month = doc["date"]["month"];
            int    day   = date["day"];
            cout << "  month: " << month << endl;
            cout << "  day: " << day << endl;
        }

        if (doc.hasField("toys") && Value::ARRAY == doc.fieldType("toys")) {
            cout << "- toys:" << endl;
            for (auto toy : doc["toys"]) {
                cout << "  " << toy << endl;
            }
        }

        cout << endl;
    }
    cout << "Done!" << endl;
} catch (const mysqlx::Error& err) {
    cout << "ERROR: " << err << endl;
    return 1;
} catch (std::exception& ex) {
    cout << "STD EXCEPTION: " << ex.what() << endl;
    return 1;
} catch (const char* ex) {
    cout << "EXCEPTION: " << ex << endl;
    return 1;
}