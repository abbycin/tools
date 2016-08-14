cmake_minimum_required(VERSION 2.6)

project(logger CXX)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(inc)
add_subdirectory(src)
add_subdirectory(test)
