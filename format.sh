#!/bin/bash

find . -iname "*.h" -exec clang-format -style=file -i {} \;
find . -iname "*.cpp" -exec clang-format -style=file -i {} \;
find . -iname "*.cc" -exec clang-format -style=file -i {} \;

