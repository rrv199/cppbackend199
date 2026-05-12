cmake_minimum_required(VERSION 3.14)
project(game_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)
add_compile_definitions(BOOST_JSON_HEADER_ONLY)

# Подключаем Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost REQUIRED COMPONENTS system)

set(SOURCES
    main.cpp
    http_server.cpp
    json_loader.cpp
    request_handler.cpp
    boost_json.cpp
    loot_generator.cpp
    player_manager.cpp
    globals.cpp
)

add_executable(game_server ${SOURCES})

target_include_directories(game_server PRIVATE .)
target_link_libraries(game_server ${Boost_LIBRARIES} ${CONAN_LIBS} pthread)

# Тесты
add_executable(game_server_tests
    tests/loot_generator_tests.cpp
    loot_generator.cpp
)
target_include_directories(game_server_tests PRIVATE . external)
target_compile_definitions(game_server_tests PRIVATE CATCH_CONFIG_MAIN)

enable_testing()
add_test(NAME LootGeneratorTests COMMAND game_server_tests)
