#include <gtest/gtest.h>
#include "urlencode.h"

// Тест 1: Пустая строка
TEST(UrlEncodeTest, EmptyString) {
    EXPECT_EQ(UrlEncode(""), "");
}

// Тест 2: Строка без служебных символов
TEST(UrlEncodeTest, NoSpecialChars) {
    EXPECT_EQ(UrlEncode("abcdefghijklmnopqrstuvwxyz"), "abcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ(UrlEncode("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    EXPECT_EQ(UrlEncode("0123456789"), "0123456789");
    EXPECT_EQ(UrlEncode("_-."), "_-.");
}

// Тест 3: Строка с пробелами
TEST(UrlEncodeTest, Spaces) {
    EXPECT_EQ(UrlEncode("Hello World"), "Hello+World");
    EXPECT_EQ(UrlEncode("  "), "++");
    EXPECT_EQ(UrlEncode("Hello   World"), "Hello+++World");
}

// Тест 4: Строка со служебными символами
TEST(UrlEncodeTest, ReservedChars) {
    EXPECT_EQ(UrlEncode("!"), "%21");
    EXPECT_EQ(UrlEncode("#"), "%23");
    EXPECT_EQ(UrlEncode("$"), "%24");
    EXPECT_EQ(UrlEncode("&"), "%26");
    EXPECT_EQ(UrlEncode("'"), "%27");
    EXPECT_EQ(UrlEncode("("), "%28");
    EXPECT_EQ(UrlEncode(")"), "%29");
    EXPECT_EQ(UrlEncode("*"), "%2A");
    EXPECT_EQ(UrlEncode("+"), "%2B");
    EXPECT_EQ(UrlEncode(","), "%2C");
    EXPECT_EQ(UrlEncode("/"), "%2F");
    EXPECT_EQ(UrlEncode(":"), "%3A");
    EXPECT_EQ(UrlEncode(";"), "%3B");
    EXPECT_EQ(UrlEncode("="), "%3D");
    EXPECT_EQ(UrlEncode("?"), "%3F");
    EXPECT_EQ(UrlEncode("@"), "%40");
    EXPECT_EQ(UrlEncode("["), "%5B");
    EXPECT_EQ(UrlEncode("]"), "%5D");
}

// Тест 5: Символы с кодами меньше 32
TEST(UrlEncodeTest, ControlChars) {
    char tab = '\t';
    char newline = '\n';
    char carriage = '\r';
    
    EXPECT_EQ(UrlEncode(std::string(1, tab)), "%09");
    EXPECT_EQ(UrlEncode(std::string(1, newline)), "%0A");
    EXPECT_EQ(UrlEncode(std::string(1, carriage)), "%0D");
    EXPECT_EQ(UrlEncode("Hello\tWorld"), "Hello%09World");
}

// Тест 6: Символы с кодом 128 и выше (Unicode/UTF-8)
TEST(UrlEncodeTest, HighChars) {
    // Русские символы в UTF-8
    std::string russian = "Привет";
    std::string expected = "%D0%9F%D1%80%D0%B8%D0%B2%D0%B5%D1%82";
    EXPECT_EQ(UrlEncode(russian), expected);
    
    // Немецкие символы
    std::string german = "üäöß";
    EXPECT_NE(UrlEncode(german), german);
}

// Тест 7: Комбинация различных символов
TEST(UrlEncodeTest, MixedChars) {
    EXPECT_EQ(UrlEncode("Hello World!"), "Hello+World%21");
    EXPECT_EQ(UrlEncode("abc*"), "abc%2A");
    EXPECT_EQ(UrlEncode("a b c"), "a+b+c");
    EXPECT_EQ(UrlEncode("Hello+World"), "Hello%2BWorld");
}

// Тест 8: Строка с цифрами и буквами, не требующими кодирования
TEST(UrlEncodeTest, Alphanumeric) {
    EXPECT_EQ(UrlEncode("Hello123"), "Hello123");
    EXPECT_EQ(UrlEncode("Test_String"), "Test_String");
}

// Тест 9: Только специальные символы
TEST(UrlEncodeTest, OnlySpecial) {
    EXPECT_EQ(UrlEncode("!@#$%^&*()"), "%21%40%23%24%25%5E%26%2A%28%29");
}

// Тест 10: Пустая строка после кодирования
TEST(UrlEncodeTest, EmptyResult) {
    EXPECT_EQ(UrlEncode(""), "");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
