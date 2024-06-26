### tools

header-only libraries written in modern C++, some are list below:


|item| description |
|-|-|
|[bloom](./bloom)| nαïve bloom filter |
|[cfg](./cfg)| hash table |
|[channel](./channel) | [Rust](https://www.rust-lang.org)-like [channel](https://doc.rust-lang.org/std/sync/mpsc/fn.channel.html) implementation based on lock-free mpsc queue |
|[coroutine](./coroutine) | coroutine impl from scratch, demonstrate how to impl your own coroutine |
|[crypt](./crypt) | toy |
|[fake_variant](./fake_variant)| tuple-like class based on multi-inherit |
|[file_sort](./file_sort)| sort numbers in a big file (bitset and divide-conquer) |
|[fm](./fm)| functor-map for storing any callable to a signal object, see [example](./fm/test.cpp) |
|[form_parser](./form_parser)| multipart/form-data parser |
|[json](./json)| nαïve json library |
|[logging](./logging)| logging library based on mpsc queue of [channel](./channel) |
|[loop_per_thread](./loop_per_thread)| test for loop per thread paradigm |
|[rbtree](./rbtree)| red-black tree implementation |
|[avl](./avl)| avl tree implementation |
|[bptree](./bptree)| in memory B+ tree implementation |
|[signal](./signal)| simple signal-slot implementation, see [ss](https://github.com/abbycin/ss) |
|[string_ext](./string_ext)| extended std::string |
|[threadpool](./threadpool)| thread pool implementation via std::thread and lock-based task queue |
|[variant](./variant) | variant implementation for C++11 |
|[optional](./optional) | optional implementation for C++11 |
|[typelist.cpp](./typelist.cpp) | loki-like typelist implemented by modern C++ |

and more...

### LICENSE
[GPLv3](./LICENSE)([with exception](https://gcc.gnu.org/onlinedocs/libstdc++/manual/license.html))
