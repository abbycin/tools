/*********************************************************
          File Name:threadpool.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Thu 04 Aug 2016 09:33:30 AM CST
**********************************************************/

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <cassert>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <vector>

namespace nm
{
  class threadpool final
  {
    public:
      threadpool(std::size_t threads = std::thread::hardware_concurrency());
      threadpool(const threadpool&) = delete;
      threadpool& operator=(const threadpool&) = delete;
      ~threadpool() noexcept;
      template<typename F, typename... Args>
      auto add_task(F&&, Args&&...)
      -> std::future<typename std::result_of<F(Args...)>::type>;
      size_t queue_size_limit();
      void set_queue_size_limit(size_t);
      void stop();
    private:
      bool is_exit;
      bool is_stop;
      size_t TASK_LIMIT;
      std::vector<std::thread> workers;
      std::queue<std::function<void()>> tasks;
      std::mutex queue_lock;
      std::condition_variable queue_cond;
  };
  threadpool::threadpool(size_t threads)
    : is_exit(false), is_stop(false), TASK_LIMIT(1000)
  {
    for(;threads > 0; --threads)
    {
      workers.emplace_back([&] {
          std::function<void()> task;
          for(;;)
          {
            {
            std::unique_lock<std::mutex> l(queue_lock);
              queue_cond.wait(l, [this]
                  { return is_stop || is_exit || !tasks.empty(); });
              if(is_stop) // caller request to stop all task.
                return;
              // normal exit, when pool is going to destroyed.
              if(is_exit && tasks.empty())
                break;
              task = move(tasks.front());
              tasks.pop();
            }
            try
            {
              task();
            }
            catch(std::exception& e)
            {
              // abort program when encounter an exception
              assert(false);
            }
          }
        }
      );
    }
  }
  threadpool::~threadpool() noexcept
  {
    try
    {
      std::unique_lock<std::mutex> l(queue_lock);
      is_exit = true;
      queue_cond.notify_all();
    }
    catch(std::system_error&)
    {
      assert(false);
    }
    for(auto& t: workers)
    {
      try
      {
        t.join();
      }
      catch(std::system_error&)
      {
        // do nothing
      }
    }
  }
  template<typename F, typename... Args>
  auto threadpool::add_task(F&& f, Args&&... args)
  -> std::future<typename std::result_of<F(Args...)>::type>
  {
    using R = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<R()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto res(task->get_future());
    std::unique_lock<std::mutex> l(queue_lock);
    if(tasks.size() >= TASK_LIMIT)
      throw std::runtime_error("task queue is full.");
    if(is_stop)
      throw std::runtime_error("add_task on stopped threadpool.");
    tasks.emplace([task]{ (*task)(); });
    queue_cond.notify_one();
    return res;
  }
  size_t threadpool::queue_size_limit()
  {
    return TASK_LIMIT;
  }
  void threadpool::set_queue_size_limit(size_t size)
  {
    std::unique_lock<std::mutex> l(queue_lock);
    auto old_limit = TASK_LIMIT;
    TASK_LIMIT = (size > 1 ? size : 1);
    // if new limit less then old one,then wake up all workers to
    // process as many tasks as they can
    if(TASK_LIMIT < old_limit)
      queue_cond.notify_all();
  }
  void threadpool::stop()
  {
    std::unique_lock<std::mutex> l(queue_lock);
    is_stop = true;
    queue_cond.notify_all();
  }
}

#endif
