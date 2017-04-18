cmake_minimum_required(VERSION 3.7)
project(logging)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror -pedantic-errors -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror -pedantic-errors")

set(SOURCE_FILES
    logging.h
    logging.cpp)

add_executable(logging_test ${SOURCE_FILES} test.cpp)
target_link_libraries(logging_test pthread)

add_library(logger SHARED ${SOURCE_FILES})
set_target_properties(logger PROPERTIES VERSION 1.0)

function(exec_install)
    set(target_path "/usr/local/include/logger")
    file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/logging.h)
    foreach(h ${HEADERS})
        install(FILES ${h} DESTINATION ${target_path})
    endforeach()
    if(EXISTS /usr/local/lib64)
        install(TARGETS logger LIBRARY DESTINATION /usr/local/lib64)
    elseif(EXISTS /usr/local/lib)
        install(TARGETS logger LIBRARY DESTINATION /usr/local/lib)
    endif()
endfunction()

exec_install()
