cmake_minimum_required(VERSION 3.7)
project(rbtree)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES test.cpp)
add_executable(rbtree ${SOURCE_FILES})
