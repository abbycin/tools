### NOTE

You need a compiler which support C++17 (at least C++11).

For those who has C++11 support, here's a trick to simualte `inline variable` of C++17. You need modifiy the [source](./logging.h)
for example:
```c++
template<typename T = void>
class Logger
{
  private:
    thread_local static StreamBuffer ss_;
};

template<typename T> thread_local StreamBuffer Logger<T>::ss_{};

#define LOG_INFO nm::Logger<>(__FILE__, __LINE__, nullptr, nm::Logger<>::INFO)
```
and you have to create a logging instance using `nm::Logger<>::create_sync()`.

### benchmark

[bench.cc](./bench.cc)

5400RPM HDD + i5 6300HQ
```
$ g++ bench.cc -pthread -O3
$ ./a.out
0.376758861s
$ du -h *.log
92M xx.log
```
speed ≈ 244 M/s

### TODO
如果glibc足够新的话，可以使用固定大小的mpsc队列来去除内存分配的开销(主要是看glibc的memcpy是否能启用指令加速)
