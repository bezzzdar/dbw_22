#ifndef __DIALOGUE_BOT_UTILS_H_INCLUDED__
#define __DIALOGUE_BOT_UTILS_H_INCLUDED__

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

namespace bot_utils {
std::vector<std::string> bad_words{"хуй"};

std::string ToLowerNoSpaces(const std::string& str) {
    setlocale(LC_CTYPE, "ru_RU.UTF-8");
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::wstring                                                    ws = converter.from_bytes(str);
    std::transform(ws.begin(),
                   ws.end(),
                   ws.begin(),
                   std::bind2nd(std::ptr_fun(&std::tolower<wchar_t>), std::locale("ru_RU.UTF-8")));

    std::remove_if(ws.begin(), ws.end(), isspace);

    std::string str2 = converter.to_bytes(ws);

    return str2;
}

bool IsValidName(const std::string& name) {
    bool is_bad = false;

    const auto string = ToLowerNoSpaces(name);

    for (const auto& w : bad_words) {
        if (string.find(w) != std::string::npos) {
            is_bad = true;
            break;
        }
    }

    return !is_bad;
}
}; // namespace bot_utils

#endif