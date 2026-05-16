cmake_minimum_required(VERSION 3.14)
project(book_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost REQUIRED)

# Настройка pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(book_manager
    src/main.cpp
)

# Все линковки в одном месте
target_link_libraries(book_manager
    ${CONAN_LIBS}
    ${Boost_LIBRARIES}
    Threads::Threads
)

target_include_directories(book_manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
