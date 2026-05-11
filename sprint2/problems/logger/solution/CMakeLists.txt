cmake_minimum_required(VERSION 3.14)

# Явно указываем компиляторы
set(CMAKE_C_COMPILER /usr/bin/g++)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

project(hello_log CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hello_log main.cpp)

target_compile_definitions(hello_log PRIVATE -DLOG_IMPLEMENTATION)
