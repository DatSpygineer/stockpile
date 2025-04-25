cmake_minimum_required(VERSION 3.30)

find_program(VCPKG_PATH NAMES vcpkg.exe)
get_filename_component(VCPKG_BASE_PATH ${VCPKG_PATH} DIRECTORY)
include(${VCPKG_BASE_PATH}/scripts/buildsystems/vcpkg.cmake)