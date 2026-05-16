cmake_minimum_required(VERSION 3.14)
project(state_serialization)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(serialization_lib STATIC
    src/state_serializer.cpp
)

target_include_directories(serialization_lib PUBLIC include)
target_link_libraries(serialization_lib ${Boost_LIBRARIES})

find_package(Boost REQUIRED COMPONENTS serialization filesystem)

# Скачиваем Catch2
file(DOWNLOAD https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp
     ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)

add_executable(serialization_tests
    tests/serialization_tests.cpp
)

target_link_libraries(serialization_tests serialization_lib pthread)
target_include_directories(serialization_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR} include)

enable_testing()
add_test(NAME SerializationTests COMMAND serialization_tests)
