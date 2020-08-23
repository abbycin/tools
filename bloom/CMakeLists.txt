cmake_minimum_required(VERSION 3.15)
project(bloom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${PROJECT_SOURCE_DIR}/murmur3)

add_subdirectory(murmur3)
add_executable(bloom bloom.h main.cpp)
target_link_libraries(bloom murmur3)