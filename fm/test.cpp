/***********************************************
        File Name: test.cpp
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 1/10/19 7:25 PM
***********************************************/

#include "functor_map.h"
#include <iostream>

std::string to_str(int x) { return std::to_string(x); }

struct Foo
{
  static std::string foo() { return __func__; }
  int operator()(int x) { return x * x; }
};

int main()
{
  using std::cout;
  using std::string;
  nm::FunctorMap fm;

  // bind
  fm.bind("lambda", [](int lhs, int rhs) { return lhs + rhs; });
  fm.bind("lambda2", [](string lhs, string rhs) { return lhs + rhs; });
  fm.bind("void_arg_res", [] { std::cout << "void\n"; });
  fm.bind("void_arg", [] { return "666"; });
  fm.bind("free_function", to_str);
  fm.bind("static_member_function", &Foo::foo);
  Foo foo;
  fm.bind("capture", [&foo](int x) { return foo(x); });

  fm.call<string>("none exist");
  cout << fm.call<int>("lambda", 1, 2) << '\n';
  cout << fm.call<string>("lambda2", string{"hello "}, string{"world"}) << '\n';
  fm.call("void_arg_res");
  cout << fm.call<const char*>("void_arg") << '\n';
  cout << fm.call<string>("free_function", 233) << '\n';
  cout << fm.call<string>("static_member_function") << '\n';
  cout << fm.call<int>("capture", 16) << '\n';
  try
  {
    fm.call<int>("capture");
  }
  catch(const std::runtime_error& e)
  {
    cout << e.what() << '\n';
  }
}