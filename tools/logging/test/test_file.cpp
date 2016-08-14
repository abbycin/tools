/*********************************************************
          File Name:test_file.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 01:02:26 PM CST
**********************************************************/

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include "file.h"

int main(int argc, char* argv[])
{
  if(argc < 2)
  {
    fprintf(stderr, "%s path\n", argv[0]);
    return 1;
  }
  const char arr[] = "1234567890abcdefghijklmnopqrstuvwxzyABCDEFGHIJKLMNOPQRSTUVWXZY\n";
  int len = strlen(arr);
  // split by size
  nm::FileCtl file(std::string(argv[1]) + "size_",
      10485760, 60 * 60 * 24, true, 3, 1024); // roll every 10M
  for(int j = 0; j < 2000000; ++j)
    file.append(arr, len);
  // split by duration
  nm::FileCtl file2(std::string(argv[1]) + "duration_",
      0, 3, true, 3, 1024); // roll every 3s
  for(int j = 0; j < 3; ++j)
  {
    for(int i = 0; i < 10000; ++i)
      file2.append(arr, len);
    sleep(3);
  }
}
