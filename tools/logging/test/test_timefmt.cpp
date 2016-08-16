/*********************************************************
          File Name:test_timefmt.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 10:52:32 AM CST
**********************************************************/

#include <iostream>
#include "timefmt.h"

int main()
{
  char buf[25] = {0};
  std::cout << nm::TimeFmt::now().to_string() << std::endl;
  std::cout << nm::TimeFmt::now().format(buf, 25) << std::endl;
  std::cout << buf << std::endl;
}
