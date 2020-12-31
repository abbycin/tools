find_package(Boost 1.61 REQUIRED system)

add_executable(server server.cpp)
target_link_libraries(server ${Boost_LIBRARIES} pthread)

add_executable(server1 server1.cpp)
target_link_libraries(server1 ${Boost_LIBRARIES} pthread)

#add_executable(client client.cpp)
#target_link_libraries(client ${Boost_LIBRARIES} pthread)
