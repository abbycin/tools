# A minimum implementation of coroutine

### Requirement

- CMake 3.10 (or any other version support C++17)
- nasm
- g++7 or clang5 (linux)
- Visual Studio 2017



### Build & Run

**Linux & macOS**
```
mkdir build && cd build
cmake .. && cmake --build .
./coroutine
```
**Windows**
```
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build .
.\Debug\coroutine.exe
```

### Doc

Please go to my [blog post](https://note.isliberty.me/2018/06/02/a-coroutine-impl/)

### Bugs

Do NOT report bug, it's just a demonstration that explain how to implement coroutine.
