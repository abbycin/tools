/*********************************************************
          File Name: test.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 26 Feb 2017 10:46:31 AM CST
**********************************************************/

#include "channel.h"
#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

void sender(Sender<size_t> tx, size_t data, size_t limit)
{
  for (; limit > 0;)
  {
    limit -= 1;
    tx.send(data);
  }
  printf("thread %lu done\n", data);
}

void receiver(Receiver<size_t> rx)
{
  std::vector<size_t> vec(3, 0);
  size_t res = 0;
  for(;;)
  {
    rx.recv(res);
    if(res < vec.size())
      vec[res] += 1;
    else
      break;
  }
  for(size_t i = 0; i < vec.size(); ++i)
    printf("thread %lu => %lu\n", i, vec[i]);
  printf("receiver done.\n");
}

auto now()
{
  return std::chrono::high_resolution_clock::now();
}

template<typename T> // no GNU extension.
auto duration(const T& dur)
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
}

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "%s num\n", argv[0]);
    return 1;
  }

  size_t num = std::stoull(argv[1]);

  auto pair = channel<size_t>();
  auto& tx = std::get<0>(pair);
  auto& rx = std::get<1>(pair);
  auto start = now();

  std::thread rcv(receiver, std::move(rx));

  std::vector<std::thread> pool;
  for(int i = 0; i < 3; ++i)
  {
    pool.emplace_back(sender, tx.clone(), i, num);
  }

  for(auto& x : pool)
    x.join();
  tx.send(309);
  rcv.join();
  auto end = now();
  auto dur = static_cast<double>(duration(end - start));
  printf("%.9f\n", dur / 1000000000);
}
