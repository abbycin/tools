# Functor Map

A container for all callable objects (functor, lamda, free function).

## Example

```c++
FunctorMap fm;
fm.bind("foo", [] { std::cout << "lambda\n"; });
fm.bind("foo", [] {}); // return false, becuase "foo" is already existed
fm.bind("bar", [](int x) { return x; });

fm.call("foo"); // print "lambda"
std::cout << fm.call<int>("bar", 233) << '\n'; // print 233
try {
  fm.call<void>("bar", "too", "bad");
} catch(const std::runtime_error& e) {
  std::cerr << e.what() << '\n'; // print parameter mismatch (return type, parameter type and parameter count)
}
```

