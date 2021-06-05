# STRUCTURE

- `bot.cpp` - main file. here lies bot's logic and initialization
- `bot_api.*` - UNUSED. it's functionality is implemented inside `bot.cpp` main switch-case
- `bot_utils.h` - utilities to adequately work with russian text and process user's answers
- `db_api.*` - api to work with database

# NOTES

it may be needed to make tweaks in `CMakeLists.txt` and headers (tg bot and cpp connector). but main functionality should remain the same
