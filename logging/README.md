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
