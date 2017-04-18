/*********************************************************
          File Name: test.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 09 Apr 2017 02:19:50 PM CST
**********************************************************/

#include <iostream>
#include <thread>
#include <vector>
#include "logging.h"

using namespace std;
using namespace nm;

int main(int argc, char* argv[])
{
  const char* path = argc > 1 ? argv[1] : "";
  const char* prefix = argc > 2 ? argv[2] : "";
  const long interval = argc > 3 ? std::stol(argv[3]) : 1000'000 * 10;

  if(!Logger::create_async(path, prefix, interval))
    return 1;

  vector<thread> tg;
  for(int i = 0; i < 4; ++i)
  {
    tg.emplace_back([] {
      for(int j = 0; j < 200; ++j)
      {
        LOG_INFO << "test info " << 233 << " "<< true;
        LOG_WARN << "test warning " << 233 << " " << false;
        LOG_DEBUG << "test debug " << -1 << " "<< true;
        LOG_ERR << "test error " << &j << " "<< true;
        LOG_FATAL << "test +1s " << 233 << " "<< false;
      }
    });
  }
  for(auto& x: tg)
  {
    try
    {
      x.join();
    }
    catch(const std::system_error& e)
    {
      cout << e.what() << endl;
    }
  }
}
