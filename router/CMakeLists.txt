cmake_minimum_required(VERSION 3.15)
project(router)

set(CMAKE_CXX_STANDARD 20)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(trie trie.cc)
add_executable(radix radix.cc)
add_executable(3way 3way.cc)
add_executable(router router.h main.cpp)