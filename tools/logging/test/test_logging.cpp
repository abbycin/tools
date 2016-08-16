/*********************************************************
          File Name:test_logging.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 04:41:54 PM CST
**********************************************************/

#include <iostream>
#include "logging.h"

int main(int argc, char* argv[])
{
  if(argc < 3)
  {
    fprintf(stderr, "%s path message ...\n", argv[0]);
    return 1;
  }
  nm::Logging::instance(argv[1], 1024 * 1024 * 10);
  int j = 2;
  for(int i = 0; i < 250000; ++i)
  {
      nm::LOG_INFO << argv[j] << "\n";
      nm::LOG_WARN << argv[j] << "\n";
      nm::LOG_DEBUG << argv[j] << "\n";
      nm::LOG_ERR << argv[j] << "\n";
  }
  nm::LOG_FATAL << " --->>> fatal error!\n";
}
