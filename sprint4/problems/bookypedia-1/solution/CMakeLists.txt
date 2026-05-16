cmake_minimum_required(VERSION 3.14)
project(bookypedia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(bookypedia
    src/main.cpp
    src/domain/author.cpp
    src/domain/book.cpp
    src/app/use_cases_impl.cpp
    src/repository/author_repository.cpp
    src/repository/book_repository.cpp
)

target_link_libraries(bookypedia
    ${CONAN_LIBS}
    ${Boost_LIBRARIES}
    Threads::Threads
)

target_include_directories(bookypedia PRIVATE src)
