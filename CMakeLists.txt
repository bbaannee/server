cmake_minimum_required(VERSION 3.2)
project(WebServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/WebServer.cpp
    src/TcpListener.cpp
    src/Logger.cpp
)

add_executable(server ${SOURCES})

target_include_directories(server PRIVATE include)

# Multithreading
find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE Threads::Threads)

# Warnings
if(MSVC)
    target_compile_options(server PRIVATE /W4)
else()
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
endif()
