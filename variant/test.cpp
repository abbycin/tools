/*********************************************************
          File Name: test.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 May 2017 09:15:08 AM CST
**********************************************************/

#include <iostream>
#include "variant.h"

using namespace std;
using namespace nm;

struct foo
{
  foo() = default;
  foo(int x, int y)
    : x_(x), y_(y) {}
  ~foo()
  { printf("--->>> %s\n", __func__); };
  void show()
  {
    printf("x: %d\ty: %d\n", x_, y_);
  }
  int x_;
  int y_;
};

struct bar
{
  bar() = default;
  virtual ~bar() { printf("--->>> %s\n", __func__); }
  virtual void foo() { printf("bar\n"); }
};

struct baz : public bar
{
  ~baz() { printf("--->>> %s\n", __func__); }
  void foo() override { printf("baz\n"); }
};

variant<std::string> make_variant()
{
  return variant<std::string>{std::string{"test move"}};
}

int main()
{
  variant<bar, int, foo, std::string, double> va{bar()};
  va.get<bar>().foo(); // print 'bar'
  va.set<int>(233); cout << va.get<int>() << '\n';
  va.set<std::string>(std::string("+1s")); cout << va.get<std::string>() << '\n';
  va.set<foo>(foo());
  va.emplace<foo>(1, 2); va.get<foo>().show(); // test emplace
  va.set<double>(2.333);
  va.set<std::string>("233333333333333333333333");
  variant<bar, int, foo, std::string, double> v(va); // test copy ctor
  cout << v.get<std::string>() << '\n';
  variant<std::string> vv(make_variant()); // test move ctor
  cout << vv.get<std::string>() << '\n';
  v = std::move(va); cout << v.get<std::string>() << '\n';
  variant<bar*> b{static_cast<bar*>(new baz{})};
  bar* ba = b.get<bar*>();
  ba->bar::foo();
  static_cast<baz*>(ba)->foo();
  delete static_cast<baz*>(ba);
  cout << (v != vv ? "v not equal vv" : " v equal vv") << '\n';
  cout << (v == va ? "v equal va" : " v not equal va") << '\n';
  variant<std::string, int> vx = "2.33";
  std::string s("-1s");
  vx = 233;
  vx = s;
  cout << s.size() << '\n'; cout << vx.get<std::string>() << '\n';
  // test call
  variant<int, float, std::string, const char*> vc = 2.33;
  vc.call([](int x) { printf("int = %d\n", x); },
          [](float x) { printf("float = %f\n", x); },
          [](std::string x) { printf("string = %s\n", x.c_str()); },
          [](const char* x) { printf("const char* = %s\n", x); });
  vc = "maybe string";
  vc.call([](std::string x) { printf("string = %s\n", x.c_str()); });
  vc.call([](const double) { printf("never print\n"); });
}
