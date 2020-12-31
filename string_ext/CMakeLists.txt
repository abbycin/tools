add_executable(string_ext test.cpp)
add_custom_command(
        TARGET string_ext POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/ip.txt
                ${CMAKE_CURRENT_BINARY_DIR}/ip.txt)