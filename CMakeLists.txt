cmake_minimum_required(VERSION 3.16)

project(qtraylib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(modules/raylib)
add_subdirectory(src)
add_subdirectory(example)