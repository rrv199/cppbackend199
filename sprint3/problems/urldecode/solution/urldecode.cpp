#include "urldecode.h"
#include <stdexcept>
#include <cctype>
#include <cstdlib>

std::string UrlDecode(std::string_view str) {
    std::string result;
    result.reserve(str.size());
    
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            // Проверяем, что есть достаточно символов для %XX
            if (i + 2 >= str.size()) {
                throw std::invalid_argument("Incomplete percent encoding");
            }
            
            // Проверяем, что следующие два символа - hex digits
            if (!std::isxdigit(static_cast<unsigned char>(str[i + 1])) ||
                !std::isxdigit(static_cast<unsigned char>(str[i + 2]))) {
                throw std::invalid_argument("Invalid percent encoding");
            }
            
            // Преобразуем hex в символ
            char hex[3] = {str[i + 1], str[i + 2], '\0'};
            char ch = static_cast<char>(std::strtol(hex, nullptr, 16));
            result.push_back(ch);
            i += 2;
        } else {
            // Не заменяем '+' на пробел, оставляем как есть
            result.push_back(str[i]);
        }
    }
    
    return result;
}
