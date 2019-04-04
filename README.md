### tools

header-only libraries written in modern C++ (11, 14, 17, 2a).


|item| describe |
|-|-|
|[cfg](./cfg)| hash table |
|[channel](./channel) | [Rust](https://www.rust-lang.org)-like [channel](https://doc.rust-lang.org/std/sync/mpsc/fn.channel.html) implementation based on lock-free mpsc queue |
|[clp](./clp) | C++17  feature test |
|[coroutine](./coroutine) | coroutine impl from scratch, demonstrate how to impl your own coroutine |
|[crypt](./crypt) | toy |
|[fake_variant](./fake_variant)| tuple-like class based on multi-inherit |
|[file_sort](./file_sort)| sort numbers in a big file (bitset and divide-conquer) |
|[fm](./fm)| functor-map for storing any callable to a signal object, see [example](./fm/test.cpp) |
|[form_parser](./form_parser)| multipart/form-data parser |
|[logging](./logging)| logging library based on mpsc queue of [channel](./channel) |
|[loop_per_thread](./loop_per_thread)| test for loop per thread paradigm |
|[rbtree](./rbtree)| red-black tree implementation |
|[signal](./signal)| simple signal-slot implementation, see [ss](https://github.com/abbycin/ss) |
|[string_ext](./string_ext)| extended std::string |
|[threadpool](./threadpool)| thread pool implementation via std::thread and lock-based task queue |
|[variant](./variant) | variant implementation for C++11 |
|[optional](./optional) | optional implementation for C++11 |
|[typelist.cpp](./typelist.cpp) | loki-like typelist implemented by modern C++ |

### LICENSE
[GPLv3](./LICENSE)([with exception](https://gcc.gnu.org/onlinedocs/libstdc++/manual/license.html))
