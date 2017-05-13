/*********************************************************
          File Name: variant.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 May 2017 09:13:27 AM CST
**********************************************************/

#ifndef VARIANT_H__
#define VARIANT_H__

#include <stdexcept>
#include <type_traits>

namespace nm
{
  namespace meta
  {
    // size less than
    template<typename T, typename U> struct size_lt
    {
      constexpr static bool value = sizeof(T) < sizeof(U);
    };

    // size greater than
    template<typename T, typename U> struct size_gt
    {
      constexpr static bool value = sizeof(T) > sizeof(U);
    };

    template<template<typename, typename> class, typename, typename...> struct type_size;
    template<template<typename, typename> class Op, typename T> struct type_size<Op, T>
    {
      constexpr static size_t value = sizeof(T);
    };

    template<template<typename, typename> class Op, typename T, typename U, typename... Rest>
    struct type_size<Op, T, U, Rest...>
    {
      constexpr static size_t value = Op<T, U>::value
                                      ? type_size<Op, T, Rest...>::value
                                      : type_size<Op, U, Rest...>::value;
    };

    template<size_t _1, size_t...> struct max_size_of;
    template<size_t _1> struct max_size_of<_1>
    {
      constexpr static size_t value = _1;
    };
    template<size_t _1, size_t _2, size_t... Rest>
    struct max_size_of<_1, _2, Rest...>
    {
      constexpr static size_t value = _1 > _2 ? max_size_of<_1, Rest...>::value
                                              : max_size_of<_2, Rest...>::value;
    };

    template<typename T, typename... Types> struct is_in;
    template<typename T> struct is_in<T>
    {
      constexpr static bool value = false;
    };
    template<typename T, typename _1ST, typename... Rest>
    struct is_in<T, _1ST, Rest...>
    {
      private:
        constexpr static bool tmp = std::is_same<T, _1ST>::value;
      public:
        constexpr static bool value = tmp ? tmp : is_in<T, Rest...>::value;
    };

    template<typename, typename...> struct index_of_type;
    template<typename T> struct index_of_type<T>
    {
      constexpr static int value = 0;
    };
    template<typename T, typename U, typename... Rest> struct index_of_type<T, U, Rest...>
    {
      private:
        constexpr static bool tmp = std::is_same<T, U>::value;
      public:
        constexpr static int value = tmp ? 0 : 1 + index_of_type<T, Rest...>::value;
    };

    template<typename...> struct variant_helper;
    template<> struct variant_helper<>
    {
      static void clear(int, void*) {}
    };
    template<typename T, typename... R> struct variant_helper<T, R...>
    {
      static void clear(int index, void* data)
      {
        if(index > 0)
        {
          index -= 1;
          variant_helper<R...>::clear(index, data);
        }
        else if(index == 0)
        {
          if(!std::is_pod<T>::value)
          {
            reinterpret_cast<T*>(data)->~T();
          }
        }
      }
    };
  }

  // save type identity
  template<typename... Rest>
  class variant
  {
      using helper = meta::variant_helper<Rest...>;
    public:
      variant() : type_index_(-1) {}
      ~variant()
      {
        helper::clear(type_index_, &data_);
      }

      template<typename T> variant(const T& rhs)
      {
        static_assert(meta::is_in<T, Rest...>::value, "invalid type");
        new(static_cast<void*>(&data_)) T(rhs);
        type_index_ = meta::index_of_type<T, Rest...>::value;
      }

      template<typename T> void set(const T& rhs)
      {
        static_assert(meta::is_in<T, Rest...>::value, "invalid type");
        helper::clear(type_index_, &data_);
        new(static_cast<void*>(&data_)) T(rhs);
        type_index_ = meta::index_of_type<T, Rest...>::value;
      }

      template<typename T> T& get()
      {
        static_assert(meta::is_in<T, Rest...>::value, "invalid type");
        if(type_index_ < 0)
          throw std::runtime_error("bad get on an empty variant");
        int tmp = meta::index_of_type<T, Rest...>::value;
        if(type_index_ != tmp)
          throw std::runtime_error("get type miss match set type");
        return *reinterpret_cast<T*>(&data_);
      }

    private:
      // or meta::max_size_of<sizeof(Rest)...>::value;
      int type_index_;
      typename std::aligned_storage<meta::type_size<meta::size_gt, Rest...>::value,
        meta::max_size_of<alignof(Rest)...>::value>::type data_;
  };

  // forbid `variant<> foo`
  template<> class variant<>
  {
    public:
      ~variant() = delete;
  };
}

#endif // VARIANT_H__
