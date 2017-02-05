/*********************************************************
          File Name:rand.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 24 Sep 2016 03:39:01 PM CST
**********************************************************/

#include <iostream>
#include <random>

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "%s num output\n", argv[0]);
    return 1;
  }
  std::mt19937 rng;
  auto n = std::stol(argv[1]);
  rng.seed(std::random_device()());
  std::uniform_int_distribution<std::mt19937::result_type> dist(1, n);
  FILE* fp = fopen(argv[2], "w");
  for(; n > 0; --n)
  {
    fprintf(fp, "%d\n", dist(rng));
  }
  fclose(fp);
}
