cmake_minimum_required(VERSION 2.6)

project(logger CXX)

if(WIN32)
  message(FATAL_ERROR "Platform not support.")
endif()

function(set_compiler compiler)
  set (CMAKE_CXX_COMPILER ${compiler} PARENT_SCOPE)
  if(compiler STREQUAL "g++")
    set (CMAKE_CXX_COMPILER_ID "GNU" PARENT_SCOPE)
  elseif(compiler STREQUAL "clang++")
    set (CMAKE_CXX_COMPILER_ID "Clang" PARENT_SCOPE)
  endif()
endfunction()

set_compiler("g++")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.3)
    message(FATAL_ERROR "Current g++ version is ${CMAKE_CXX_COMPILER_VERSION}, required version is 5.3 or above.")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(WARNING "Require clang++ 3.8 or newer.")
else()
  message(FATAL_ERROR "Compiler not support.")
endif()

set (CMAKE_CXX_FLAGS "-std=gnu++0x")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(inc)
add_subdirectory(src)
add_subdirectory(test)

function (install_headers)
  set(target_path "/usr/include/logger")
  file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/inc/*.h)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/inc/meta DESTINATION ${target_path})
  foreach(h ${HEADERS})
    install(FILES ${h} DESTINATION ${target_path})
  endforeach()
endfunction()

install_headers()
