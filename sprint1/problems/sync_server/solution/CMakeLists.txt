cmake_minimum_required(VERSION 3.14)

# Принудительно задаём C и C++ компилятор
if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER /usr/bin/g++ CACHE PATH "C compiler" FORCE)
    set(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "" FORCE)
endif()
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER /usr/bin/g++ CACHE PATH "C++ compiler" FORCE)
    set(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "" FORCE)
endif()

project(hello CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

find_package(Boost REQUIRED COMPONENTS system)

add_executable(hello main.cpp)

target_link_libraries(hello Boost::system pthread)
