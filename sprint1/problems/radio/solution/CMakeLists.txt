cmake_minimum_required(VERSION 3.14)
project(radio)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(radio radio.cpp)

target_link_libraries(radio Boost::system pthread)
