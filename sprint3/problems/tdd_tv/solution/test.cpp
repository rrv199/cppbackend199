#include <boost/test/unit_test.hpp>
#include "tv.h"

BOOST_AUTO_TEST_SUITE(TV)

// Тест 1: Телевизор по умолчанию выключен
BOOST_AUTO_TEST_CASE(is_off_by_default) {
    TV tv;
    BOOST_CHECK(!tv.IsTurnedOn());
}

// Тест 2: После включения телевизор включен
BOOST_AUTO_TEST_CASE(turned_on_after_power_on) {
    TV tv;
    tv.TurnOn();
    BOOST_CHECK(tv.IsTurnedOn());
}

// Тест 3: После выключения телевизор выключен
BOOST_AUTO_TEST_CASE(turned_off_after_power_off) {
    TV tv;
    tv.TurnOn();
    tv.TurnOff();
    BOOST_CHECK(!tv.IsTurnedOn());
}

// Тест 4: При первом включении канал = 1
BOOST_AUTO_TEST_CASE(channel_is_1_after_first_power_on) {
    TV tv;
    tv.TurnOn();
    BOOST_CHECK_EQUAL(tv.GetChannel(), 1);
}

// Тест 5: При выключении канал запоминается
BOOST_AUTO_TEST_CASE(channel_is_remembered_after_power_off) {
    TV tv;
    tv.TurnOn();
    tv.SelectChannel(5);
    tv.TurnOff();
    tv.TurnOn();
    BOOST_CHECK_EQUAL(tv.GetChannel(), 5);
}

// Тест 6: Невозможно выбрать канал, когда телевизор выключен
BOOST_AUTO_TEST_CASE(cannot_select_channel_when_off) {
    TV tv;
    tv.SelectChannel(10);  // Должно игнорироваться
    tv.TurnOn();
    BOOST_CHECK_EQUAL(tv.GetChannel(), 1);  // Все еще канал 1
}

// Тест 7: Можно выбрать канал только в диапазоне 1-999
BOOST_AUTO_TEST_CASE(can_select_channel_only_in_range) {
    TV tv;
    tv.TurnOn();
    
    tv.SelectChannel(0);
    BOOST_CHECK_EQUAL(tv.GetChannel(), 1);  // Не изменился
    
    tv.SelectChannel(1000);
    BOOST_CHECK_EQUAL(tv.GetChannel(), 1);  // Не изменился
    
    tv.SelectChannel(500);
    BOOST_CHECK_EQUAL(tv.GetChannel(), 500);
}

BOOST_AUTO_TEST_SUITE_END()
