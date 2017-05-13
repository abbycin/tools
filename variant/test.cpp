/*********************************************************
          File Name: test.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 May 2017 09:15:08 AM CST
**********************************************************/

#include <iostream>
#include "variant.h"

struct foo
{
  foo() = default;
  ~foo()
  { printf("--->>> %s\n", __func__); };
};

struct bar
{
  bar() = default;
  ~bar() { printf("--->>> %s\n", __func__); }
  void foo() { printf("bar\n"); }
};

int main()
{
  using namespace std;
  using namespace nm;
  variant<bar, int, foo, std::string> va{bar()};
  va.get<bar>().foo(); // print 'bar'
  va.set<int>(233); cout << va.get<int>() << '\n';
  va.set<std::string>(std::string("+1s")); cout << va.get<std::string>() << '\n';
  va.set<foo>(foo());
}