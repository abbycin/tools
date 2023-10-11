add_executable(swiss_set_test swiss_set_test.cc swisstable.h)
target_include_directories(swiss_set_test PRIVATE ${PROJECT_SOURCE_DIR})

add_executable(swiss_map_test swiss_map_test.cc)