cmake_minimum_required(VERSION 3.10)
project(bookypedia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем Conan (если есть)
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

# Добавляем исполняемый файл
add_executable(bookypedia src/main.cpp)

# Линкуем библиотеки
target_link_libraries(bookypedia
    ${CONAN_LIBS}
    pthread
)

# Если Conan не использовался, пробуем найти библиотеки в системе
if(NOT CONAN_LIBS)
    find_library(PQXX_LIB pqxx REQUIRED)
    find_library(PQ_LIB pq REQUIRED)
    find_library(BOOST_LIB boost_system REQUIRED)
    
    target_link_libraries(bookypedia
        ${PQXX_LIB}
        ${PQ_LIB}
        ${BOOST_LIB}
        pthread
    )
    
    target_include_directories(bookypedia PRIVATE
        /usr/include/postgresql
    )
endif()

# Устанавливаем флаги компиляции
target_compile_options(bookypedia PRIVATE -Wall -Wextra -g)
