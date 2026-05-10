cmake_minimum_required(VERSION 3.14)

# Отключаем проверку C компилятора
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(hello CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

# Подключаем Conan
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

find_package(Boost REQUIRED)

if(DEFINED Boost_INCLUDE_DIRS)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

add_executable(hello src/main.cpp)

target_link_libraries(hello Boost::system pthread)
