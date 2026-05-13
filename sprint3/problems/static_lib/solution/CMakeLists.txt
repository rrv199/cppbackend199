cmake_minimum_required(VERSION 3.14)
project(game_lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Статическая библиотека
add_library(game_model STATIC
    src/json_loader.cpp
)

target_include_directories(game_model PUBLIC include)

# Тесты
add_executable(game_model_tests 
    tests/loot_generator_tests.cpp
)

target_link_libraries(game_model_tests game_model pthread)
target_include_directories(game_model_tests PRIVATE include)

# Скачиваем Catch2
file(DOWNLOAD https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp
     ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)
target_include_directories(game_model_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(game_model_tests PRIVATE CATCH_CONFIG_MAIN)

enable_testing()
add_test(NAME LootGeneratorTests COMMAND game_model_tests)
