cmake_minimum_required(VERSION 3.10)
project(bookypedia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем исполняемый файл
add_executable(bookypedia src/main.cpp)

# Находим libpqxx (используем pkg-config)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)
pkg_check_modules(PQ REQUIRED libpq)

# Добавляем библиотеки
target_link_libraries(bookypedia
    ${PQXX_LIBRARIES}
    ${PQ_LIBRARIES}
    pthread
)

# Добавляем пути для include
target_include_directories(bookypedia PRIVATE
    ${PQXX_INCLUDE_DIRS}
    ${PQ_INCLUDE_DIRS}
)

# Устанавливаем флаги компиляции
target_compile_options(bookypedia PRIVATE -Wall -Wextra -g)
