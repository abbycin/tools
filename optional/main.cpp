/*********************************************************
          File Name: main.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Thu 04 Apr 2019 01:42:50 PM DST
**********************************************************/

#include "optional.h"
#include <iostream>

struct Bar
{
  explicit Bar(int z) : z_{ z } {}
  virtual ~Bar() {}
  int z_;
};

struct Foo : Bar
{
  Foo(int x, int y, int z) : Bar{ z }, x_ { x }, y_{ y } {}
  int x_;
  int y_;
};

int main()
{
  std::cerr << std::boolalpha;

  Optional<Bar> b{ Bar{1} };
  Optional<Bar> h = b;
  Optional<Foo> o{ Foo{ 1, 2, 3 } };

  Optional<Foo> l{Foo{ 3, 2, 1 }};
  std::cerr << (bool)l << '\t' << o->x_ << '\n';

  o = std::move(l);

  std::cerr << (bool)l << '\t' << o->x_ << '\n';

  l = o;

  std::cerr << (bool)l << '\t' << o->x_ << '\n';

  l = Foo{ 2, 2, 2 };
  o = std::move(l);
  std::cerr << (bool)l << '\t' << o->x_ << '\n';

  Optional<Foo> m = o;
  std::cerr << (bool)m << '\t' << m->x_ << '\n';
}