cmake_minimum_required(VERSION 3.14)
project(htmldecode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(htmldecode_lib htmldecode.cpp)

add_executable(test_htmldecode test_simple.cpp)
target_link_libraries(test_htmldecode htmldecode_lib)

enable_testing()
add_test(NAME HtmlDecodeTest COMMAND test_htmldecode)
