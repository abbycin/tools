/*********************************************************
          File Name:test.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Mon 21 Nov 2016 09:55:05 PM CST
**********************************************************/

#include <iostream>
#include <vector>
#include "variant.h"

using namespace std;

struct NonCopyable
{
  NonCopyable(int x): data(x) {}
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator= (const NonCopyable&) = delete;
  int data;
};

int main()
{
  // ignore extra `int`
  nm::FakeVariant<int, const char*, NonCopyable, double, int, vector<int>> va;
  va.set<int>(10);
  va.set<const char*>("+1s");
  va.set<double>(3.09);
  va.set<NonCopyable>(42);
  va.set<vector<int>>({1, 2, 3});
  cout << va.get<int>() << endl;
  cout << va.get<const char*>() << endl;
  cout << va.get<double>() << endl;
  cout << va.get<NonCopyable>().data << endl;
  for(const auto x: va.get<vector<int>>())
    cout << x << " ";
  cout << endl;
}
