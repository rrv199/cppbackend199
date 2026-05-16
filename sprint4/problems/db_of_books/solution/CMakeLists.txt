cmake_minimum_required(VERSION 3.14)
project(book_manager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем libpqxx
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Ищем nlohmann_json (header-only)
find_package(nlohmann_json CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
    # Если не найден, скачиваем
    include(FetchContent)
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
    )
    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
        FetchContent_Populate(json)
    endif()
    set(NLOHMANN_JSON_INCLUDE_DIR ${json_SOURCE_DIR})
else()
    get_target_property(NLOHMANN_JSON_INCLUDE_DIR nlohmann_json::nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)
endif()

add_executable(book_manager
    src/main.cpp
)

target_link_libraries(book_manager
    ${PQXX_LIBRARIES}
    pthread
)

target_include_directories(book_manager PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${NLOHMANN_JSON_INCLUDE_DIR}
)

target_compile_options(book_manager PRIVATE ${PQXX_CFLAGS_OTHER})
