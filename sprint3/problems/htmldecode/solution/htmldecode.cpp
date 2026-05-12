#include "htmldecode.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <cctype>
#include <algorithm>

std::string HtmlDecode(std::string_view str) {
    std::string result;
    result.reserve(str.size());
    
    std::unordered_map<std::string, char> entities = {
        {"lt", '<'},
        {"gt", '>'},
        {"amp", '&'},
        {"apos", '\''},
        {"quot", '"'}
    };
    
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '&') {
            // Поиск конца мнемоники (буквы)
            size_t end = i + 1;
            while (end < str.size() && std::isalpha(static_cast<unsigned char>(str[end]))) {
                ++end;
            }
            
            if (end > i + 1) {
                std::string entity_name(str.substr(i + 1, end - i - 1));
                bool has_semicolon = (end < str.size() && str[end] == ';');
                
                // Проверяем, что все символы в одном регистре
                bool all_upper = true;
                bool all_lower = true;
                for (char c : entity_name) {
                    if (!std::isupper(static_cast<unsigned char>(c))) all_upper = false;
                    if (!std::islower(static_cast<unsigned char>(c))) all_lower = false;
                }
                
                // Ищем мнемонику (только если все символы в одном регистре)
                bool found = false;
                if (all_upper || all_lower) {
                    std::string lower_name = entity_name;
                    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
                                   [](unsigned char c) { return std::tolower(c); });
                    
                    auto it = entities.find(lower_name);
                    if (it != entities.end()) {
                        result.push_back(it->second);
                        i = end - 1;
                        if (has_semicolon) i++;
                        found = true;
                    }
                }
                if (found) continue;
            }
        }
        result.push_back(str[i]);
    }
    
    return result;
}
