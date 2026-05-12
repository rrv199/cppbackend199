#include <iostream>
#include <string>
#include <stdexcept>
#include "urldecode.h"

#define TEST(expr) \
    if (!(expr)) { \
        std::cerr << "Test failed: " << #expr << " in " << __FILE__ << " line " << __LINE__ << std::endl; \
        return 1; \
    }

#define TEST_THROWS(expr) \
    try { \
        expr; \
        std::cerr << "Test failed: expected exception not thrown: " << #expr << " in " << __FILE__ << " line " << __LINE__ << std::endl; \
        return 1; \
    } catch (const std::invalid_argument&) { \
    } catch (...) { \
        std::cerr << "Test failed: wrong exception type for " << #expr << " in " << __FILE__ << " line " << __LINE__ << std::endl; \
        return 1; \
    }

int main() {
    // Тест 1: Пустая строка
    TEST(UrlDecode("") == "");
    
    // Тест 2: Строка без %-последовательностей
    TEST(UrlDecode("Hello World!") == "Hello World!");
    
    // Тест 3: Строка с валидными %-последовательностями
    TEST(UrlDecode("Hello%20World%21") == "Hello World!");
    
    // Тест 4: Валидные %-последовательности в разном регистре
    TEST(UrlDecode("%2F") == "/");
    TEST(UrlDecode("%2f") == "/");
    
    // Тест 5: Строка с символом '+' (должен остаться '+')
    TEST(UrlDecode("Hello+World") == "Hello+World");
    
    // Тест 6: Смешанные случаи
    TEST(UrlDecode("Hello%20World+%21") == "Hello World+!");
    
    // Тест 7: Зарезервированные символы (должны остаться без изменений)
    TEST(UrlDecode("!#$&'()*+,/:;=?@[]") == "!#$&'()*+,/:;=?@[]");
    
    // Тест 8: Кодирование символов
    TEST(UrlDecode("%41%42%43") == "ABC");
    
    // Тест 9: Невалидные %-последовательности
    TEST_THROWS(UrlDecode("Hello%2GWorld"));
    
    // Тест 10: Неполные %-последовательности
    TEST_THROWS(UrlDecode("Hello%2"));
    TEST_THROWS(UrlDecode("Hello%"));
    TEST_THROWS(UrlDecode("%"));
    
    // Тест 11: Процент в конце строки
    TEST_THROWS(UrlDecode("test%"));
    
    // Тест 12: Русские символы в URL encoding
    TEST(UrlDecode("%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82") == "Привет");
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
