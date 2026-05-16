cmake_minimum_required(VERSION 3.14)
project(bookypedia CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Добавляем исполняемый файл
add_executable(bookypedia src/main.cpp)

# Линкуем библиотеки
target_link_libraries(bookypedia ${CONAN_LIBS})

# Создаём скрипт для создания ссылки после сборки
add_custom_command(TARGET bookypedia POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/bin/bookypedia
        ${CMAKE_BINARY_DIR}/bookypedia
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
