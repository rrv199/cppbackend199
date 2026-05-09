cmake_minimum_required(VERSION 3.14)
project(game_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(game_server
    src/http_server.cpp
    src/json_loader.cpp
    src/main.cpp
    src/request_handler.cpp
    src/boost_json.cpp
)

target_include_directories(game_server PRIVATE src)
target_link_libraries(game_server pthread)
