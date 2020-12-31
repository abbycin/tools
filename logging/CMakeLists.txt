set(SOURCE_FILES logging.h test.cpp)

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
