cmake_minimum_required(VERSION 3.10)
project(sum CXX)  # ← Указываем, что нужен только C++

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем компилятор явно (опционально, для GitHub Actions)
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER "g++")
endif()

add_executable(sum sum.cpp)
