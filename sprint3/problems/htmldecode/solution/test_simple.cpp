#include <iostream>
#include <string>
#include "htmldecode.h"

#define TEST(expr) \
    if (!(expr)) { \
        std::cerr << "Test failed: " << #expr << " in " << __FILE__ << " line " << __LINE__ << std::endl; \
        return 1; \
    }

int main() {
    // Тест 1: Пустая строка
    TEST(HtmlDecode("") == "");
    
    // Тест 2: Строка без HTML-мнемоник
    TEST(HtmlDecode("Hello World") == "Hello World");
    TEST(HtmlDecode("No entities here") == "No entities here");
    TEST(HtmlDecode("123456") == "123456");
    
    // Тест 3: Строка с HTML-мнемониками
    TEST(HtmlDecode("&lt;") == "<");
    TEST(HtmlDecode("&gt;") == ">");
    TEST(HtmlDecode("&amp;") == "&");
    TEST(HtmlDecode("&apos;") == "'");
    TEST(HtmlDecode("&quot;") == "\"");
    
    // Тест 4: Мнемоники в верхнем регистре
    TEST(HtmlDecode("&LT;") == "<");
    TEST(HtmlDecode("&GT;") == ">");
    TEST(HtmlDecode("&AMP;") == "&");
    TEST(HtmlDecode("&APOS;") == "'");
    TEST(HtmlDecode("&QUOT;") == "\"");
    
    // Тест 5: Смешанный регистр (не должны декодироваться)
    TEST(HtmlDecode("&lT;") == "&lT;");
    TEST(HtmlDecode("&aPos;") == "&aPos;");
    
    // Тест 6: Мнемоники без точки с запятой
    TEST(HtmlDecode("&lt") == "<");
    TEST(HtmlDecode("&gt") == ">");
    TEST(HtmlDecode("&amp") == "&");
    TEST(HtmlDecode("&apos") == "'");
    TEST(HtmlDecode("&quot") == "\"");
    
    // Тест 7: Мнемоники с точкой с запятой
    TEST(HtmlDecode("&lt;") == "<");
    TEST(HtmlDecode("&gt;") == ">");
    TEST(HtmlDecode("&amp;") == "&");
    TEST(HtmlDecode("&apos;") == "'");
    TEST(HtmlDecode("&quot;") == "\"");
    
    // Тест 8: Несколько мнемоник
    TEST(HtmlDecode("&lt;&gt;") == "<>");
    TEST(HtmlDecode("&amp;&quot;") == "&\"");
    TEST(HtmlDecode("Johnson&amp;Johnson") == "Johnson&Johnson");
    
    // Тест 9: Мнемоники в тексте
    TEST(HtmlDecode("M&amp;M&apos;s") == "M&M's");
    TEST(HtmlDecode("&lt;Hello&gt;") == "<Hello>");
    TEST(HtmlDecode("Start &amp; End") == "Start & End");
    
    // Тест 10: Неизвестные мнемоники
    TEST(HtmlDecode("&unknown;") == "&unknown;");
    TEST(HtmlDecode("&abracadabra") == "&abracadabra");
    TEST(HtmlDecode("&123;") == "&123;");
    
    // Тест 11: Частичные мнемоники
    TEST(HtmlDecode("&lt") == "<");
    TEST(HtmlDecode("&") == "&");
    TEST(HtmlDecode("&l") == "&l");
    TEST(HtmlDecode("&lt;&") == "<&");
    
    // Тест 12: Вложенные мнемоники не декодируются дважды
    TEST(HtmlDecode("&amp;lt;") == "&lt;");
    TEST(HtmlDecode("&amp;amp;") == "&amp;");
    
    // Тест 13: Регистронезависимость для одинакового регистра
    TEST(HtmlDecode("&LT") == "<");
    TEST(HtmlDecode("&AMP;") == "&");
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
