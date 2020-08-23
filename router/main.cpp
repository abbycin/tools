/***********************************************
        File Name: main.cpp
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 3/29/20 1:08 PM
***********************************************/

#include "router.h"

using std::cout;

void test(Router& r, std::string path)
{
  auto [handler, args, ok] = r.get(path);
  if(ok)
  {
    handler(args);
  }
  else
  {
    cout << "not found: " << path << '\n';
  }
}

void add(Router& r, std::string path, Handler handler)
{
  auto e = r.add(path, handler);
  if(e)
  {
    cout << "add " << path << " failed: " << *e << '\n';
  }
}

std::ostream& operator<<(std::ostream& os, const Pairs& m)
{
  for(auto [k, v]: m)
  {
    os << k << " => " << v << "\n";
  }
  return os;
}

template<typename... Args>
void print(Args&&... args)
{
  ((cout << args << ' '), ...);
  cout << '\n';
}

int main()
{
  {
    Router r{};
    add(r, "/foo", [](auto) { print("/foo"); });
    add(r, "/foo/", [](auto) { print("/foo/"); });
    test(r, "/foo");
    test(r, "/foo/");
  }
  {
    Router r{};
    add(r, "/foo", [](auto) { print("/foo"); });
    add(r, "/foo/:bar", [](auto m) { print(m); });
    add(r, "/foo/", [](auto) { print("/foo/"); });
    test(r, "/foo");
    test(r, "/foo/");
    test(r, "/foo/elder");
  }
  {
    Router r{};
    add(r, "/foo/*bar", [](auto m) { print(m); });
    add(r, "/foo", [](auto) { print("/foo"); });
    test(r, "/foo");
    test(r, "/foo/are you ok?");
  }
  {
    Router r{};
    add(r, "/foo", [](auto) { print("/foo"); });
    add(r, "/foo/", [](auto) { print("/foo/"); });
    add(r, "/foo/*bar", [](auto m) { print(m); });
    test(r, "/foo");
    test(r, "/foo/");
    test(r, "/foo/elder");
  }
  {
    Router r{};
    add(r, "/foo/:bar", [](auto m) { print("param", m); });
    add(r, "/foo/*bar", [](auto m) { print("wild", m); });
    test(r, "/foo/elder");
    test(r, "/foo/-1s");
  }
  {
    Router r{};
    add(r, "/foo/*bar", [](auto m) { cout << "wild bar: " << m["bar"] << '\n'; });
    add(r, "/foo/:bar", [](auto m) { print("param", m); });
    test(r, "/foo/elder");
    test(r, "/foo/-1s");
  }
  {
    Router r{};
    add(r, "/foobar/*bar/baz", [](auto m) { print("wild", m); });
    add(r, "/foo/bar*baz", [](auto m) { print("wild", m); });
  }
  {
    Router r{};
    add(r, "/foo/*bar", [](auto m) { print("wild", m); });
    add(r, "/foo/bar/*baz", [](auto m) { print("wild", m); });
    test(r, "/foo/elder");
    test(r, "/foo/-1s");
  }
  {
    Router r{};
    add(r, "/foo", [](auto m) { print("/foo"); });
    add(r, "/foo/", [](auto m) { print("/foo/"); });
    add(r, "/foo/:name", [](auto m) { print(m); });
    add(r, "/foo/:name/:age", [](auto m) { print(m); });
    test(r, "/foo/elder/"); // expect failed
    test(r, "/foo/elder/+1s");
  }
  {
    Router r{};
    add(r, "/foo", [](auto) { print("/foo"); });
    add(r, "/foo/", [](auto m) { print("/foo/"); });
    add(r, "/foo/:name", [](auto m) { print(m); });
    add(r, "/foo/:name/*age", [](auto m) { print(m); });
    test(r, "/foo/elder/"); // expect ok
    test(r, "/foo/elder/+1s");
  }
  {
    Router r{};
    add(r, "/foo", [](auto) { print("/foo"); });
    add(r, "/foo/", [](auto m) { print("/foo/"); });
    add(r, "/foo/bar", [](auto) { print("/foo/bar"); });
    add(r, "/foo/:name", [](auto) {});
    add(r, "/foo/:name/*age", [](auto) {});
  }
  {
    Router r{};
    add(r, "/:foo", [](auto m) { print(m); });
    add(r, "/:foo/elder/:bar", [](auto m) { print(m); });
    add(r, "/:foo/+1s", [](auto m) { print(m); });
    test(r, "/elder");
    test(r, "/jiang/elder/+1s");
    test(r, "/elder1/+1s");
  }
}
