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
const std::vector<std::string> bad_words{
    "хуй",
    "хуе",
    "хуё",
    "хуя",
    "хуи",
    "соси",
    "сосат",
    "сасат",
    "лох",
    "пидр",
    "пидор",
    "пидар",
    "член",
    "сука",
    "сучий",
    "дибил",
    "дебил",
};

std::string ToLowerNoSpaces(const std::string& str) {
    setlocale(LC_CTYPE, "ru_RU.UTF-8");
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::wstring ws = converter.from_bytes(str);
    std::transform(ws.begin(),
                   ws.end(),
                   ws.begin(),
                   std::bind2nd(std::ptr_fun(&std::tolower<wchar_t>), std::locale("ru_RU.UTF-8")));

    // std::remove_if(ws.begin(), ws.end(), isspace);
    ws.erase(std::remove(ws.begin(), ws.end(), L' '), ws.end());
    ws.erase(std::remove(ws.begin(), ws.end(), L'\t'), ws.end());
    ws.erase(std::remove(ws.begin(), ws.end(), L'\n'), ws.end());
    ws.erase(std::remove(ws.begin(), ws.end(), L'\r'), ws.end());

    return converter.to_bytes(ws);
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