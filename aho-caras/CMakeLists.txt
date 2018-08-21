cmake_minimum_required(VERSION 3.8)
project(aho_caras)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
add_executable(aho_caras ${SOURCE_FILES})