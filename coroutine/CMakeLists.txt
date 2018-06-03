cmake_minimum_required(VERSION 3.10)
project(coroutine)

set(CMAKE_CXX_STANDARD 17)
enable_language(ASM_NASM)

add_executable(coroutine main.cpp switch_stack.asm)
