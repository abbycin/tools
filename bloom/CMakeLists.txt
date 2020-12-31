include_directories(${CMAKE_CURRENT_SOURCE_DIR}/murmur3)
add_subdirectory(murmur3)
add_executable(bloom bloom.h main.cpp)
target_link_libraries(bloom murmur3)