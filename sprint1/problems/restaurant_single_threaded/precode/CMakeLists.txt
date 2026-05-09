cmake_minimum_required(VERSION 3.14)
project(restaurant)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(restaurant src/main.cpp)

target_link_libraries(restaurant Boost::system pthread)
