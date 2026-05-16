cmake_minimum_required(VERSION 3.14)
project(find_return)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(find_return_lib STATIC
    src/collision_detector.cpp
)

target_include_directories(find_return_lib PUBLIC include)

add_executable(find_return_demo src/main.cpp)
target_link_libraries(find_return_demo find_return_lib)

# Тесты
file(DOWNLOAD https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp
     ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)

add_executable(find_return_tests tests/find_return_tests.cpp)
target_link_libraries(find_return_tests find_return_lib)
target_include_directories(find_return_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR} include)

enable_testing()
add_test(NAME FindReturnTests COMMAND find_return_tests)
