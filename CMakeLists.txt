cmake_minimum_required(VERSION 3.16)
project(BookstoreManagement)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

add_executable(code
    src/main.cpp
    src/command.cpp
    src/account.cpp
    src/book.cpp
    src/log.cpp
)

target_include_directories(code PRIVATE ${CMAKE_SOURCE_DIR}/src)
