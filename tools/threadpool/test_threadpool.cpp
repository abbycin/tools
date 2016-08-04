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
  };
  nm::threadpool pool;
  pool.add_task(f, 1);
  pool.add_task(f, 2);
  pool.add_task(f, 3);
  pool.add_task(f, 4);
  pool.add_task(f, 5);
  pool.add_task(f, 6);
  pool.add_task(f, 7);
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
  this_thread::sleep_for(chrono::seconds(1));
  pool.stop();
}
