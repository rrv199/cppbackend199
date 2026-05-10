cmake_minimum_required(VERSION 3.14)

project(hello CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

# Подключаем Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)  # Добавляем поиск pthread

# Добавляем пути к заголовкам Boost
include_directories(${Boost_INCLUDE_DIRS})

add_executable(hello src/main.cpp)

# Линкуем с библиотеками
target_link_libraries(hello 
    PRIVATE 
    Threads::Threads
    ${Boost_LIBRARIES}
)
