# Channel

A C++ implementation of **MPSC** (multi-producer single-consumer) [channel](./channel.h).

see [mpsc.rs](./mpsc.rs).

-----
`rustc -C opt-level=3 mpsc.rs -o a.out
` [rustc 1.20.0-nightly]
```
./a.out 10000000
thread 0 done
thread 2 done
thread 1 done
thread 0 => 10000000
thread 1 => 10000000
thread 2 => 10000000
receiver done.
3.118793157
```
`g++ -std=c++17 test.cpp -pthread -O3` [g++ (SUSE Linux) 7.1.1 20170629]
```
thread 0 done
thread 2 done
thread 1 done
thread 0 => 10000000
thread 1 => 10000000
thread 2 => 10000000
receiver done.
3.111892014
```
`g++ -std=c++17 test.cpp -pthread -O3 -DJEMALLOC -ljemalloc` [g++ (SUSE Linux) 7.1.1 20170629]
```
./a.out 10000000
thread 2 done
thread 0 done
thread 1 done
thread 0 => 10000000
thread 1 => 10000000
thread 2 => 10000000
receiver done.
2.148938502
```
