cmake_minimum_required(VERSION 3.14)
project(state_serialization)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS serialization)

add_library(serialization_lib STATIC
    src/state_serializer.cpp
)

target_include_directories(serialization_lib PUBLIC include)
target_link_libraries(serialization_lib 
    Boost::serialization
)

# Скачиваем Catch2
file(DOWNLOAD https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp
     ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)

add_executable(serialization_tests
    tests/serialization_tests.cpp
)

target_link_libraries(serialization_tests 
    serialization_lib
    Boost::serialization
    pthread
)

target_include_directories(serialization_tests PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR} 
    include
)

target_compile_definitions(serialization_tests PRIVATE CATCH_CONFIG_MAIN)

enable_testing()
add_test(NAME SerializationTests COMMAND serialization_tests)

# Также создаем game_server как алиас
add_custom_target(game_server ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:serialization_tests> ${CMAKE_BINARY_DIR}/game_server
    DEPENDS serialization_tests
)
