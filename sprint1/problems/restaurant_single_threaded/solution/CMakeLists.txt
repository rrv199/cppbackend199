cmake_minimum_required(VERSION 3.14)

# Отключаем проверку C компилятора
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(restaurant CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost REQUIRED COMPONENTS system)

# Добавляем пути к заголовкам Boost
include_directories(${Boost_INCLUDE_DIRS})

add_executable(restaurant src/main.cpp)

target_link_libraries(restaurant Boost::system pthread)
