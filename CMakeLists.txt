cmake_minimum_required(VERSION 3.13)

project(Single LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE on)

set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/distribution)

add_library(sqlite3 SHARED
        ${distribution_DIR}/sqlite/sqlite3.c)


add_executable(main main.c)

target_include_directories(main PRIVATE
        ${distribution_DIR}/sqlite)

target_link_libraries(main
        sqlite3)
# "C:\Program Files\JetBrains\CLion 2018.3.3\bin\cmake\win\bin\cmake.exe"