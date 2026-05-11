cmake_minimum_required(VERSION 3.14)
project(game_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)
add_compile_definitions(BOOST_JSON_HEADER_ONLY)

find_package(Boost REQUIRED)

set(SOURCES
    src/main.cpp
    src/http_server.cpp
    src/json_loader.cpp
    src/request_handler.cpp
    src/boost_json.cpp
)

add_executable(game_server ${SOURCES})

target_include_directories(game_server PRIVATE src)
target_link_libraries(game_server ${Boost_LIBRARIES} pthread)
