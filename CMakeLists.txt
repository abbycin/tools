cmake_minimum_required(VERSION 3.25)
project(tools)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror -Wextra -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror -Wextra")

file(GLOB items RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/*)
foreach(item ${items})
    string(REGEX MATCH "^[^\.].*" dot ${item})
    string(REGEX MATCH ".*build.*" build ${item})
    if(("${dot}" STREQUAL "") OR (NOT ("${build}" STREQUAL "")) OR (${item} STREQUAL "loop_per_thread"))
        continue()
    endif()
    if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${item})
        message(STATUS ${item})
        add_subdirectory(${PROJECT_SOURCE_DIR}/${item})
    endif()
endforeach()
