cmake_minimum_required(VERSION 3.14)
project(gather_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Скачиваем Catch2 v2 header
file(DOWNLOAD https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp
     ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)

add_executable(collision_tests
    tests/collision-detector-tests.cpp
    collision_detector.cpp
)

target_include_directories(collision_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(collision_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_test(NAME CollisionTests COMMAND collision_tests)
