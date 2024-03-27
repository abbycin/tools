pointer stability is not guaranteed

a good hash function is required (see [absl::Hash](https://abseil.io/docs/cpp/guides/hash))

benchmark

```shell
& abby @ chaos in ~/tools/swisstable (master %)
λ g++ -std=gnu++23 swiss_set_test.cc -I ../ -O2 -DNDEBUG -static 
& abby @ chaos in ~/tools/swisstable (master %)
λ ./a.out
-----------      int insert ------------
  std::unordered_set => 91.921114ms
               swiss => 14.606567ms
-----------      int search ------------
  std::unordered_set => 42.302741ms
               swiss => 5.604564ms
unordered_set cap 1056323 size 786959 load_factor 0.744998
swisstable    cap 1048575 size 786959 load_factor 0.750503
-----------   string insert ------------
  std::unordered_set => 137.117048ms
               swiss => 59.452718ms
-----------   string search ------------
  std::unordered_set => 87.527250ms
               swiss => 39.907157ms
unordered_set cap 1056323 size 787000 load_factor 0.745037
swisstable    cap 1048575 size 787000 load_factor 0.750542
```
