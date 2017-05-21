# A Simple Single-Header Only `variant`

## features
- single header only library
- simple `set/operator=` operations
- also a `emplace` method
- two ways for visiting underlying object

## requirement
A complier which support c++11 or later (actually, if old c++ standard is not considered, it will be more easy to implement).

## how to use
See [test.cpp](./test.cpp).

## bugs
As you can see, there's only one [test](./test.cpp), but I don't think there are serious bugs, what I really concern is **Undefined Behavior** in this [library](./variant.h).

## note
the `call` member function is only tested for a bancuh of lambdas, maybe it will work for `std::function` or any other `functors`.
