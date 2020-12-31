find_package(Threads)
add_executable(mpsc test.cpp)
target_link_libraries(mpsc Threads::Threads)