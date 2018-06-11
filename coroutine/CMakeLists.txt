cmake_minimum_required(VERSION 3.10)
project(coroutine)

set(CMAKE_CXX_STANDARD 17)
enable_language(ASM_NASM)

set(SWITCH_STACK switch_stack.asm)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  # since cmake don't work properly in macOS
  # if this failed, to get object file by typing:
  # nasm -fmacho64 --prefix _ switch_stack.asm
  set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} --prefix _")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(SWITCH_STACK switch_stack_win.asm)
endif()

add_executable(coroutine coroutine.h main.cpp ${SWITCH_STACK})
