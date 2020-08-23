/*********************************************************
          File Name:bloom_filter.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 23 Aug 2020 03:51:01 PM CST
**********************************************************/

#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <cmath>
#include <functional>
#include <string_view>
#include <vector>
#include "MurmurHash3.h"
#include <iostream>

class BloomFilter
{
public:
  template<typename... Args>
  static void print(Args&&... args)
  {
    ((std::cout << args << ' '), ...);
    std::cout << '\n';
  }

  // n: collection size
  // r: expect false positive ratio
  BloomFilter(size_t n, double r)
  {
    n_ = n;
    nb_ = -1 * (n_ * std::log(r)) / std::pow(std::log(2), 2);
    k_ = std::ceil(std::log(2) * nb_ / n_);
    init();
    print("size:", bits_.size(), nb_);
  }

  [[nodiscard]] double estimate() const { return std::pow(1 - std::exp(-((double)n_ * k_ / nb_)), k_); }

  void add(std::string_view x)
  {
    for(auto& f: hash_)
    {
      auto h = f(x) % nb_;
      size_t span = h / width_;
      size_t slot = h % width_;
      bits_[span] |= slot;
    }
  }

  bool test(std::string_view x)
  {
    for(auto& f: hash_)
    {
      auto h = f(x) % nb_;
      size_t span = h / width_;
      size_t slot = h % width_;
      if(!(bits_[span] & slot))
      {
        return false;
      }
    }
    return true;
  }

private:
  using hash_func = std::function<int64_t(std::string_view)>;
  constexpr static size_t width_ = sizeof(uint64_t) * 8;
  size_t n_;
  size_t nb_;
  size_t k_;
  std::vector<uint64_t> bits_;
  std::vector<hash_func> hash_;

  void init()
  {
    bits_.resize(nb_ / width_ + 1); // ceil
    for(size_t i = 0; i < k_; ++i)
    {
      hash_.emplace_back([i](std::string_view x) -> uint64_t {
        uint32_t h = 0;
        uint32_t l = 0;
        hash(x, &h, &l);
        uint64_t r = h;
        r <<= sizeof(uint32_t);
        r |= (i + 1) * l;
        return r;
      });
    }
  }

  static void hash(std::string_view x, uint32_t* h, uint32_t* l)
  {
    MurmurHash3_x86_32(x.data(), x.size(), 7, l);
    MurmurHash3_x86_32(x.data(), x.size(), 17, h);
  }
};

#endif // BLOOM_FILTER_H