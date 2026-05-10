cmake_minimum_required(VERSION 3.14)

# Отключаем проверку C компилятора
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(game_server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(BOOST_BEAST_USE_STD_STRING_VIEW)

# Включаем header-only режим Boost.Json
add_compile_definitions(BOOST_JSON_HEADER_ONLY)

# Подключаем Conan
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
endif()

find_package(Boost REQUIRED COMPONENTS system)

# Добавляем пути к заголовкам Boost
if(DEFINED Boost_INCLUDE_DIRS)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# Добавляем все исходные файлы
add_executable(game_server
    src/main.cpp
    src/http_server.cpp
    src/json_loader.cpp
    src/request_handler.cpp
    src/boost_json.cpp
)

target_link_libraries(game_server Boost::system pthread)
