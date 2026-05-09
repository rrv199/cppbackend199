cmake_minimum_required(VERSION 3.14)
project(cafeter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(cafeter
    src/main.cpp
)

target_include_directories(cafeter PRIVATE src)
target_link_libraries(cafeter Boost::system pthread)
