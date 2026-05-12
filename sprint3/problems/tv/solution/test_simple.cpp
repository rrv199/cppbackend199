#include <iostream>
#include <cassert>
#include "tv.h"

#define TEST(expr) \
    if (!(expr)) { \
        std::cerr << "Test failed: " << #expr << " in " << __FILE__ << " line " << __LINE__ << std::endl; \
        return 1; \
    }

int main() {
    // Тест 1: Телевизор по умолчанию выключен
    {
        TV tv;
        TEST(!tv.IsTurnedOn());
    }
    
    // Тест 2: У выключенного телевизора нет канала
    {
        TV tv;
        TEST(!tv.GetChannel().has_value());
    }
    
    // Тест 3: После включения телевизор включен и показывает канал 1
    {
        TV tv;
        tv.TurnOn();
        TEST(tv.IsTurnedOn());
        TEST(tv.GetChannel().value() == 1);
    }
    
    // Тест 4: После выключения телевизор выключен и не показывает канал
    {
        TV tv;
        tv.TurnOn();
        tv.TurnOff();
        TEST(!tv.IsTurnedOn());
        TEST(!tv.GetChannel().has_value());
    }
    
    // Тест 5: Выбор канала
    {
        TV tv;
        tv.TurnOn();
        tv.SelectChannel(5);
        TEST(tv.GetChannel().value() == 5);
    }
    
    // Тест 6: Выбор канала только в диапазоне 1-999
    {
        TV tv;
        tv.TurnOn();
        tv.SelectChannel(0);
        TEST(tv.GetChannel().value() == 1);
        tv.SelectChannel(1000);
        TEST(tv.GetChannel().value() == 1);
        tv.SelectChannel(500);
        TEST(tv.GetChannel().value() == 500);
    }
    
    // Тест 7: Выбор предыдущего канала
    {
        TV tv;
        tv.TurnOn();
        tv.SelectChannel(5);
        tv.SelectChannel(7);
        tv.SelectPreviousChannel();
        TEST(tv.GetChannel().value() == 5);
        tv.SelectPreviousChannel();
        TEST(tv.GetChannel().value() == 7);
    }
    
    // Тест 8: Выбор предыдущего канала когда выключен
    {
        TV tv;
        tv.SelectPreviousChannel();  // Должно игнорироваться
        tv.TurnOn();
        TEST(tv.GetChannel().value() == 1);
    }
    
    // Тест 9: Запоминание канала после выключения
    {
        TV tv;
        tv.TurnOn();
        tv.SelectChannel(42);
        tv.TurnOff();
        tv.TurnOn();
        TEST(tv.GetChannel().value() == 42);
    }
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
