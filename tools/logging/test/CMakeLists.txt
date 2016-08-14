cmake_minimum_required(VERSION 2.6)

include_directories(${PROJECT_SOURCE_DIR}/inc)

add_executable(file_test test_file.cpp)
target_link_libraries(file_test logger)

add_executable(scopedlog_test test_scopedlog.cpp)
target_link_libraries(scopedlog_test logger)

add_executable(stream_test test_stream.cpp)
target_link_libraries(stream_test logger)

add_executable(timefmt_test test_timefmt.cpp)
target_link_libraries(timefmt_test logger)

add_executable(logging_test test_logging.cpp)
target_link_libraries(logging_test logger)
