cmake_minimum_required(VERSION 3.31)
project(stockpile C)

set(CMAKE_C_STANDARD 11)

find_package(ZLIB REQUIRED)

add_library(stockpile STATIC src/stockpile.c)
target_include_directories(stockpile PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)
target_link_libraries(stockpile PUBLIC ZLIB::ZLIB)