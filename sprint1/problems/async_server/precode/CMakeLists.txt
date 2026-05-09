cmake_minimum_required(VERSION 3.14)
project(hello_async)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(hello_async
    src/http_server.cpp
    src/main.cpp
)

target_include_directories(hello_async PRIVATE src)
target_link_libraries(hello_async Boost::system pthread)
