cmake_minimum_required(VERSION 3.14)
project(logger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(logger main.cpp)

target_compile_definitions(logger PRIVATE -DLOG_IMPLEMENTATION)
