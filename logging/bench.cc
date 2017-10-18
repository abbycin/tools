#include <thread>
#include <vector>
#include "logging.h"

class ThreadGroup
{
  public:
    ThreadGroup()
      : tg_{}
    {}

    ~ThreadGroup()
    {
      this->join_all();
    }

    template<typename F> void spwan(F&& f)
    {
      tg_.emplace_back(std::forward<F>(f));
    }

    void join_all()
    {
      try
      {
        for(auto& t: tg_)
        {
          if(t.joinable())
            t.join();
        }
      }
      catch(...)
      {
        // swallow
      }
    }

  private:
    std::vector<std::thread> tg_;
};

void foo()
{
  for(int i = 0; i < 50000; ++i)
  {
    log_info() << i << ' ' << "abcdefghijklmnopqrst" << ' ' << &i;
    log_debug() << i << ' ' << "abcdefghijklmnopqrst" << ' ' << &i;
    log_warn() << i << ' ' << "abcdefghijklmnopqrst" << ' ' << &i;
    log_err() << i << ' ' << "abcdefghijklmnopqrst" << ' ' << &i;
    log_fatal() << i << ' ' << "abcdefghijklmnopqrst" << ' ' << &i;
  }
}

auto now()
{
  return std::chrono::high_resolution_clock::now();
}

template<typename T>
auto duration(const T& dur)
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
}

int main()
{
  nm::Logger::create_async();
  ThreadGroup tg;
  auto start = now();
  for(size_t i = 0; i < 4; ++i)
  {
    tg.spwan(foo);
  }
  tg.join_all();
  nm::Logger::join();
  auto end = now();
  auto dur = static_cast<double>(duration(end - start));
  printf("%.9fs\n", dur / 1000000000);
}
