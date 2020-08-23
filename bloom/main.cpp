/*********************************************************
          File Name:main.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 23 Aug 2020 03:51:10 PM CST
**********************************************************/

#include "bloom.h"

int main()
{
  using namespace std::string_view_literals;
  using namespace std;
  BloomFilter bl{100, 0.001};
  cout << bl.estimate() << '\n';
  vector<string_view> vs;
  vs.emplace_back("are");
  vs.emplace_back("are you");
  vs.emplace_back("are you ok");
  vs.emplace_back("are you ok?");

  vector<string_view> ha;
  ha.emplace_back("are ");
  ha.emplace_back("are you ");
  ha.emplace_back("are you ok ");

  for(auto x: vs)
  {
    bl.add(x);
  }

  cout << boolalpha;
  for(auto x: vs)
  {
    BloomFilter::print(x, "=>", bl.test(x));
  }

  BloomFilter::print("==========================");

  for(auto x: ha)
  {
    BloomFilter::print(x, "=>", bl.test(x));
  }
}