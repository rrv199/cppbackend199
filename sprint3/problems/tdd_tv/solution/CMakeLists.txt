cmake_minimum_required(VERSION 3.14)
project(tdd_tv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tv_lib tv.cpp)

add_executable(test_tv test_simple.cpp)
target_link_libraries(test_tv tv_lib)

enable_testing()
add_test(NAME TVTest COMMAND test_tv)
