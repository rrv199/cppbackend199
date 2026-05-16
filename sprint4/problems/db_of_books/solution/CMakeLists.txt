cmake_minimum_required(VERSION 3.14)
project(book_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(book_manager
    src/main.cpp
)

target_link_libraries(book_manager
    ${CONAN_LIBS}
    ${Boost_LIBRARIES}
    Threads::Threads
)

target_include_directories(book_manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# После сборки копируем бинарник в корень build
add_custom_command(TARGET book_manager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:book_manager>
        ${CMAKE_BINARY_DIR}/book_manager
)
