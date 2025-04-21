cmake_minimum_required(VERSION 4.0)
project(stockpile C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(STP_ARCH x64)
else()
    set(STP_ARCH x86)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin_${STP_ARCH}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin_${STP_ARCH}/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin_${STP_ARCH}/${CMAKE_BUILD_TYPE})

find_package(ZLIB REQUIRED)

add_library(stockpile STATIC src/stockpile.c)
target_include_directories(stockpile PUBLIC ${CMAKE_CURRENT_LIST_DIR}/inc)
target_link_libraries(stockpile PUBLIC ZLIB::ZLIB)

add_subdirectory(test)
add_subdirectory(tools)