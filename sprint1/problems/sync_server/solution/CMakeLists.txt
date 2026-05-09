cmake_minimum_required(VERSION 3.11)

project(hello CXX)

set(CMAKE_CXX_STANDARD 20)

# Подключаем настройки Conan (если они есть)
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(hello main.cpp)

target_link_libraries(hello PRIVATE Threads::Threads ${Boost_LIBRARIES})
