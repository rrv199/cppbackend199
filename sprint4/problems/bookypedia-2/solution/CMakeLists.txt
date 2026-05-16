cmake_minimum_required(VERSION 3.14)
project(bookypedia CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем Conan (обязательно ДО find_package)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Conan уже предоставляет все необходимые библиотеки и пути
# Не нужно вызывать find_package(Boost) или find_package(Threads)

# Добавляем исполняемый файл
add_executable(bookypedia src/main.cpp)

# Линкуем всё, что предоставил Conan
target_link_libraries(bookypedia ${CONAN_LIBS})
