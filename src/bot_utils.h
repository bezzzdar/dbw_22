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
    //todo: продумать расширение диапазона не тех символов
const std::vector<std::string> bad_words{
    "hui",   "huy",   "хуй",   "хуе",  "хуё",   "хуя",   "хуи",   "sosi",
    "sosat", "sasi",  "sasat", "соси", "сосат", "сасат", "лох",   "пидр",
    "пидор", "пидар", "член",  "сука",  "дибил", "дебил",
};

const std::vector<int> funny_numbers{
    69,
    1488,
    1337,
};

std::string ToLowerNoSpaces(const std::string& str) {
    setlocale(LC_CTYPE, "ru_RU.UTF-8");
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::wstring                                                    ws = converter.from_bytes(str);
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

std::string NoSpaces(const std::string& str) {
    setlocale(LC_CTYPE, "ru_RU.UTF-8");
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::wstring                                                    ws = converter.from_bytes(str);

    ws.erase(std::remove(ws.begin(), ws.end(), L' '), ws.end());
    ws.erase(std::remove(ws.begin(), ws.end(), L'\t'), ws.end());
    ws.erase(std::remove(ws.begin(), ws.end(), L'\n'), ws.end());
    ws.erase(std::remove(ws.begin(), ws.end(), L'\r'), ws.end());

    return converter.to_bytes(ws);
}

bool IsValidName(const std::string& name) {
    const auto string = ToLowerNoSpaces(name);

    int i = 0;
    for(i = 0; i<(int)string.length();i++)
    {
        if(isdigit(string[i]) )  
        {
            return false;
        }
        
        if(isalpha(string[i]))
        {
            return true;
        }
    }

    for (const auto& w : bad_words) {
        if (string.find(w) != std::string::npos) {
            
            return false;
        }
    }

    return true;
}

bool IsValidSchool(const int number) {
    for (const auto& num : funny_numbers) {
        if (number == num) {
            return false;
        }
    }

    return number > 0;
}

bool IsValidGrade(const int number) {
    if(number <=6 || number > 11) {
            return false;
            }
    for (const auto& num : funny_numbers) {
        if (number == num) {
          
            return false;
            
        }
    }

    return number > 0;
}

std::vector<std::string> Parse(const std::string& s, const char delim) {
    std::istringstream iss(s);
    std::string        item;

    std::vector<std::string> tokens{};

    while (std::getline(iss, item, delim)) {
        tokens.push_back(item);
    }

    return tokens;
}

}; // namespace bot_utils

#endif