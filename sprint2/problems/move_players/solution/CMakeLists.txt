cmake_minimum_required(VERSION 3.14)
project(game_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)
add_compile_definitions(BOOST_JSON_HEADER_ONLY)

# Conan setup
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

find_package(Boost REQUIRED)

set(SOURCES
    main.cpp
    http_server.cpp
    json_loader.cpp
    request_handler.cpp
    boost_json.cpp
)

add_executable(game_server ${SOURCES})

target_include_directories(game_server PRIVATE .)
target_link_libraries(game_server ${Boost_LIBRARIES} pthread)
