cmake_minimum_required(VERSION 3.7)
project(variant)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES variant.h test.cpp)
add_executable(variant ${SOURCE_FILES})