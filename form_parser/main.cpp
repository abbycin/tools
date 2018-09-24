#include <iostream>
#include <fstream>
#include "form_parser.h"

long get_file_size(const std::string& path)
{
  std::ifstream in{path, std::ios::binary};
  in.seekg(in.cur, in.end);
  long res = in.tellg();
  in.close();
  return res;
}

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    std::cerr << argv[0] << " file\n";
    return 1;
  }
  nm::FormParser parser;
  parser.set_env("---------------------------299951448578295660888714340", "/tmp");

  std::ifstream in{argv[1], std::ios::binary};
  if(!in.is_open())
  {
    std::cerr << "can't open file: " << argv[1] << '\n';
    return 1;
  }
  char buf[1024]{0};
  long n = 0;
  while(true)
  {
    in.read(buf, sizeof(buf));
    n = in.gcount();
    if(n == 0)
    {
      break;
    }
    parser.consume(buf, n);
    if(parser.is_error())
    {
      std::cerr << parser.err_string() << '\n';
      in.close();
      return 1;
    }
  }
  in.close();
  auto& texts = parser.text_map();
  for(auto& iter: texts)
  {
    std::cerr << iter.first << ": " << iter.second << '\n';
  }
  auto& files = parser.file_map();
  for(auto& iter: files)
  {
    std::cerr << iter.first << ": " << iter.second << " size: " << get_file_size(iter.second) << '\n';
  }
}