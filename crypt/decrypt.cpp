/*********************************************************
          File Name: decrypt.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 17 Dec 2016 04:02:01 PM CST
**********************************************************/

#include <iostream>
#include <fstream>
#include "crypto.h"

void cb(int magic, std::string& s)
{
  for(auto& x: s)
    x -= magic;
}

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "%s magic path\n", argv[0]);
    return 1;
  }
  int magic = 0;
  try
  {
    magic = std::stoi(argv[1]);
  }
  catch(const std::logic_error& e)
  {
    std::cerr << "Invalid magic number\n";
    return 1;
  }
  std::ifstream in(argv[2]);
  if(!in.is_open())
  {
    std::cerr << "Can't open file: " << argv[2] << std::endl;
    return 1;
  }
  std::string s;
  char c = '\0';
  in.read(&c, 1);
  in.read(&c, 1);
  in.read(&c, 1);
  std::getline(in, s);
  in.close();
  Crypto crypto;
  auto res = crypto.decode(s, magic, cb);
  std::cout << res << std::endl;
}
