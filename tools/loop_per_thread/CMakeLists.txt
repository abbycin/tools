cmake_minimum_required(VERSION 3.6)
project(loop_per_thread CXX)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -Wall")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
find_package(Boost 1.61 REQUIRED system)

add_executable(server server.cpp)
target_link_libraries(server ${Boost_LIBRARIES} pthread)

add_executable(server1 server1.cpp)
target_link_libraries(server1 ${Boost_LIBRARIES} pthread)

add_executable(client client.cpp)
target_link_libraries(client ${Boost_LIBRARIES} pthread)
