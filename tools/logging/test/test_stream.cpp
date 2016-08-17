/*********************************************************
          File Name:test_stream.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 08:20:13 PM CST
**********************************************************/

#include <iostream>
#include "meta/stream_wrapper.h"

int main()
{
  nm::meta::BufferStream ss;
  std::string str{"-str-"};
  int n = 309;
  long l = 309;
  float f = 309.1992;
  double d = 1992.18980309;
  unsigned int un = 309;
  unsigned long ul = 309;
  for(int i = 0; i < 1; ++i)
  {
    ss << str << "_" << n << "_" << l << "_" << f
      << "_" << d << "_" << un << "_" << ul << " "
      << &ul << " " << true <<"#" << '\n';
  }
  std::cout << ss.buffer();
}
