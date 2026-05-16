cmake_minimum_required(VERSION 3.10)
project(bookypedia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим библиотеки в системе
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Добавляем исполняемый файл
add_executable(bookypedia src/main.cpp)

# Линкуем библиотеки
target_link_libraries(bookypedia
    pqxx
    pq
    ${Boost_LIBRARIES}
    Threads::Threads
)

# Устанавливаем флаги компиляции
target_compile_options(bookypedia PRIVATE -Wall -Wextra -g)

# Добавляем пути для include
target_include_directories(bookypedia PRIVATE
    ${Boost_INCLUDE_DIRS}
)
