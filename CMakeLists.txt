cmake_minimum_required(VERSION 3.29)
project(Tetris C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

set(SDL2_PATH "C:/Users/Simone/CLionProjects/Games/SDL2-devel-2.30.9-mingw")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(Tetris main.c
        constants.h
        gameFunctions.h
)

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})