cmake_minimum_required(VERSION 3.14)

# Отключаем проверку C компилятора
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_C_COMPILER /usr/bin/g++)

# Явно указываем, что проект только на C++
project(hello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(hello main.cpp)

target_link_libraries(hello Boost::system pthread)
