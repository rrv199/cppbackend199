cmake_minimum_required(VERSION 3.14)
project(leave_game CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Добавляем исполняемый файл
add_executable(leave_game 
    main.cpp
)

# Линкуем библиотеки (системные)
target_link_libraries(leave_game
    pqxx
    pq
    pthread
)

# Добавляем директории для include
target_include_directories(leave_game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
