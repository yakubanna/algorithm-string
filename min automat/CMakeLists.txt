cmake_minimum_required(VERSION 3.8)
project(min_automat)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(min_automat ${SOURCE_FILES})