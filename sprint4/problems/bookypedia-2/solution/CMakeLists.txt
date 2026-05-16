cmake_minimum_required(VERSION 3.14)
project(bookypedia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(bookypedia
    src/main.cpp
)

target_link_libraries(bookypedia
    ${CONAN_LIBS}
    ${Boost_LIBRARIES}
    Threads::Threads
)

target_include_directories(bookypedia PRIVATE src)

add_custom_command(TARGET bookypedia POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:bookypedia>
        ${CMAKE_BINARY_DIR}/bookypedia
)
