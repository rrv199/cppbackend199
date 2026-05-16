cmake_minimum_required(VERSION 3.10)
project(bookypedia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Устанавливаем выходную директорию прямо здесь, до Conan
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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
    
    target_link_libraries(bookypedia
        ${PQXX_LIB}
        ${PQ_LIB}
        pthread
    )
    
    target_include_directories(bookypedia PRIVATE
        /usr/include/postgresql
    )
endif()

# Устанавливаем флаги компиляции
target_compile_options(bookypedia PRIVATE -Wall -Wextra -g)

# Устанавливаем пути для поиска заголовочных файлов Boost (header-only)
find_package(Boost REQUIRED)
target_include_directories(bookypedia PRIVATE ${Boost_INCLUDE_DIRS})

# Создаем символическую ссылку для совместимости (на всякий случай)
add_custom_command(TARGET bookypedia POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/bookypedia
        ${CMAKE_BINARY_DIR}/bin/bookypedia
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
