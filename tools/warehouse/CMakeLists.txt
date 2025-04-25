cmake_minimum_required(VERSION 3.30)

project(warehouse C CXX)

find_package(wxWidgets REQUIRED COMPONENTS net core base aui)
if(wxWidgets_USE_FILE) # not defined in CONFIG mode
    include(${wxWidgets_USE_FILE})
endif()

if (WIN32)
    set(WAREHOUSE_WIN32_SOURCES warehouse.rc)
endif()

if (NOT DEFINED WAREHOUSE_USE_MDI)
    if (WIN32)
        set(WAREHOUSE_USE_MDI 1)
    else()
        set(WAREHOUSE_USE_MDI 0)
    endif()
endif()

file(GLOB WAREHOUSE_FORM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/forms/*.hpp ${CMAKE_CURRENT_LIST_DIR}/forms/*.cpp)

add_executable(warehouse WIN32 main.cpp ${WAREHOUSE_FORM_SOURCES} ${WAREHOUSE_WIN32_SOURCES})
target_link_libraries(warehouse PRIVATE ${wxWidgets_LIBRARIES} stockpile)
target_compile_definitions(warehouse PRIVATE -DWAREHOUSE_USE_MDI=${WAREHOUSE_USE_MDI})