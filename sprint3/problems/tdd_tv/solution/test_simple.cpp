#include <iostream>
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
    
    // Тест 2: После включения телевизор включен
    {
        TV tv;
        tv.TurnOn();
        TEST(tv.IsTurnedOn());
    }
    
    // Тест 3: После выключения телевизор выключен
    {
        TV tv;
        tv.TurnOn();
        tv.TurnOff();
        TEST(!tv.IsTurnedOn());
    }
    
    // Тест 4: При первом включении канал = 1
    {
        TV tv;
        tv.TurnOn();
        TEST(tv.GetChannel() == 1);
    }
    
    // Тест 5: При выключении канал запоминается
    {
        TV tv;
        tv.TurnOn();
        tv.SelectChannel(5);
        tv.TurnOff();
        tv.TurnOn();
        TEST(tv.GetChannel() == 5);
    }
    
    // Тест 6: Невозможно выбрать канал, когда телевизор выключен
    {
        TV tv;
        tv.SelectChannel(10);  // Должно игнорироваться
        tv.TurnOn();
        TEST(tv.GetChannel() == 1);  // Все еще канал 1
    }
    
    // Тест 7: Можно выбрать канал только в диапазоне 1-999
    {
        TV tv;
        tv.TurnOn();
        
        tv.SelectChannel(0);
        TEST(tv.GetChannel() == 1);  // Не изменился
        
        tv.SelectChannel(1000);
        TEST(tv.GetChannel() == 1);  // Не изменился
        
        tv.SelectChannel(500);
        TEST(tv.GetChannel() == 500);
    }
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
