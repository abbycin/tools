/*********************************************************
          File Name:test_threadpool.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Thu 04 Aug 2016 10:43:26 AM CST
**********************************************************/

#include <chrono>
#include <exception>
#include <iostream>
#include "threadpool.h"

using namespace std;

int main()
{
  auto f = [](int x)
  {
    this_thread::sleep_for(chrono::seconds(2));
    printf("%d\n", x * x);
    return (x*x);
  };
  nm::threadpool pool;
  pool.add_task(f, 1);
  pool.add_task(f, 2);
  pool.add_task(f, 3);
  pool.pause();
  pool.add_task(f, 4);
  pool.add_task(f, 5);
  pool.add_task(f, 6);
  auto fut = pool.add_task(f, 7);
  cout << "task queue size limit: " << pool.queue_size_limit() << endl;
  pool.set_queue_size_limit(2);
  cout << "task queue size limit: " << pool.queue_size_limit() << endl;
  try
  {
    pool.add_task(f, 8);
    pool.add_task(f, 9);
    pool.add_task(f, 10);
  }
  catch(runtime_error& e)
  {
    cout << e.what() << endl;
  }
  pool.start();
  pool.wait();
  this_thread::sleep_for(chrono::seconds(1));
  pool.stop();
  if(pool.valid())
    cout << "get: " << fut.get() << endl;
  this_thread::sleep_for(chrono::seconds(3));
  cout << "--------------------------------------------------------\n";
  auto f2 = [](int x)
  {
    cout << "tid: " << this_thread::get_id() << "\tdata: " << (x * 2) << endl;
  };
  nm::threadpool pool2(std::launch::deferred);
  pool2.add_task(f2, -1);
  pool2.add_task(f2, -2);
  pool2.add_task(f2, -3);
  pool2.add_task(f2, -4);
  pool2.start();
  pool2.wait();
  pool2.stop();
  try
  {
    pool2.add_task(f2, -4);
  }
  catch(runtime_error& e)
  {
    cout << "Exception: " << e.what() << endl;
  }
  nm::threadpool pool3(std::launch::deferred);
  pool3.pause();  // set `is_start_` flag in destructor
  pool3.wait();
  cout << "done.\n";
}
