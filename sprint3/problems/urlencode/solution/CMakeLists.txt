cmake_minimum_required(VERSION 3.14)
project(urlencode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим GoogleTest
find_package(GTest REQUIRED)

# Основная библиотека
add_library(urlencode_lib urlencode.cpp)

# Исполняемый файл для тестов
add_executable(test_urlencode test.cpp)

target_link_libraries(test_urlencode urlencode_lib GTest::GTest GTest::Main)

# Включаем тесты
enable_testing()
add_test(NAME UrlEncodeTest COMMAND test_urlencode)
