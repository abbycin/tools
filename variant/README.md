# A Simple Single-Header Only `variant`

## features
- single header only library
- simple `set/operator=` operations
- also a `emplace` method
- four ways for visiting underlying object

## requirement
A complier which support c++11 or later (actually, if old c++ standard is not considered, it will be more easy to implement).

## how to use
#### a simple example
```c++
// create a variant and set value to `"233"`
nm::variant<int, string, double> va = "233";
va = 2; // reset value to 2, note, user should ensure no implicit conversion
va.set<int>(2); // reset value to 2 for type `int` explicitly
// get value of type `int`, if no value is set,`std::runtime_error` will be thrown
va.get<int>();
auto cp = va; // make a copy
va.clear(); // clear underlying object
cp.call([](int x) { cout << x << '\n';  }); // call a function for underlying type
va.emplace<string>("233"); // construct a string object
// if you don't know current underlying type, you can do this
va.call([](int x) { cout << x;  },
        [](string& x) { cout << x; },
        [](double x) { cout << x;  });

// or
struct op : public nm::variant_visitor<string>
{
  string operator() (const string& s) { return s;  }
  string operator() (int x) { return to_string(x);  }
  string operator() (double x) { return to_string(x);  }

};
cout << va.apply(op{}) << '\n'; // print 233

// or
struct op2
{
  string operator() (const string& s) { return s;  }
  string operator() (int x) { return to_string(x);  }
  string operator() (double x) { return to_string(x);  }

};
va = 2.33;
cout << va.apply<string>(op2{}) << '\n'; // print 2.330000
// or
auto visitor = nm::make_overload(
    [](int x) { cout << x;  },
    [](const string& s) { cout << s;  },
    [](double x) { cout << x;  }
);
va.apply<void>(visitor);
cout << '\n';
```
refer to [test.cpp](./test.cpp) for details.

## bugs
As you can see, there's only one [test](./test.cpp), but I don't think there are serious bugs.

## note
the `call` member function is only tested for lambdas, it may work for `std::function` or any other `functor`.
