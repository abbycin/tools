cmake_minimum_required(VERSION 3.7)
project(logging)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror -pedantic-errors -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror -pedantic-errors")

set(SOURCE_FILES
    logging.h test.cpp)

add_executable(logging_test ${SOURCE_FILES})
target_link_libraries(logging_test pthread)

function(exec_install)
    set(target_path "/usr/local/include/logger")
    file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/logging.h)
    foreach(h ${HEADERS})
        install(FILES ${h} DESTINATION ${target_path})
    endforeach()
endfunction()

exec_install()
