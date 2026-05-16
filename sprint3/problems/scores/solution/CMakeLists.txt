cmake_minimum_required(VERSION 3.14)
project(scores)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(scores_lib STATIC
    src/collision_detector.cpp
    src/game_handler.cpp
)

target_include_directories(scores_lib PUBLIC include)

# Тесты
file(DOWNLOAD https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp
     ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)

add_executable(scores_tests
    tests/scores_tests.cpp
)

target_link_libraries(scores_tests scores_lib)
target_include_directories(scores_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR} include)

enable_testing()
add_test(NAME ScoresTests COMMAND scores_tests)
