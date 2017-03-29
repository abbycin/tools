# A toy command line parser

Some C++17's features are list below:

 - std::optional
 - if constexpr
 - guaranteed copy elision
 - if init


for `std::optional`, see this example:
```c++
#include <iostream>
#include <optional>

using namespace std;

int bar(int x)
{
  if(x != 0)
    return x;
  return {};
}

optional<int> foo(int x)
{
  return {bar(x)};
}

int main()
{
  if(foo(0))
    cout << "oops!\n";
}
```
this program will print `oops!`. in this scenario, `std::pair<T, bool>` is a better choice.

**NOTE:** GCC 7 is required!
