cmake_minimum_required(VERSION 3.14)

# Отключаем проверку C компилятора
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(cafeteria CXX)  # Проект называется cafeteria

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем Conan
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

find_package(Boost REQUIRED COMPONENTS system)

# Добавляем пути к заголовкам Boost
if(DEFINED Boost_INCLUDE_DIRS)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(cafeteria src/main.cpp)  # Цель называется cafeteria

target_link_libraries(cafeteria Boost::system pthread)
