cmake_minimum_required(VERSION 3.30)

project(stockpile_test C CXX)

add_executable(stockpile_test main.cpp)
target_link_libraries(stockpile_test PRIVATE stockpile)