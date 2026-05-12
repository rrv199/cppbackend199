cmake_minimum_required(VERSION 3.14)
project(tv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Основная программа
add_executable(tv main.cpp controller.cpp tv.cpp menu.cpp)

# Тесты без внешних зависимостей
add_executable(test_tv test_simple.cpp tv.cpp)
enable_testing()
add_test(NAME TVTests COMMAND test_tv)
