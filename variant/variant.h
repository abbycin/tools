/*********************************************************
          File Name: variant.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 May 2017 09:13:27 AM CST
**********************************************************/

#ifndef VARIANT_H__
#define VARIANT_H__

#include <cstring>
#include <functional>
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

    template<typename, typename...> struct helper_and;
    template<typename _1> struct helper_and<_1>
    {
      constexpr static bool value = _1::value;
    };
    template<typename _1, typename _2, typename... R> struct helper_and<_1, _2, R...>
    {
      constexpr static bool value = _1::value ? helper_and<_2, R...>::value : _1::value;
    };

    template<typename, typename...> struct helper_or;
    template<typename _1> struct helper_or<_1>
    {
      constexpr static bool value = _1::value;
    };
    template<typename _1, typename _2, typename... R> struct helper_or<_1, _2, R...>
    {
      constexpr static bool value = _1::value ? _1::value : helper_or<_2, R...>::value;
    };

    template<typename T> struct helper_not
    {
      constexpr static bool value = !T::value;
    };

    template<typename... Rest> struct is_valid_variant_types;
    template<> struct is_valid_variant_types<>
    {
      constexpr static bool value = false;
    };
    template<typename T> struct is_valid_variant_types<T>
    {
      constexpr static bool value = helper_and<
        helper_not<std::is_same<T, void>>,
        helper_not<std::is_const<T>>,
        helper_not<std::is_reference<T>>>::value;
    };
    template<typename T, typename U> struct is_valid_variant_types<T, U>
    {
      constexpr static bool value = helper_and<
        is_valid_variant_types<T>,
        is_valid_variant_types<U>>::value;
    };
    template<typename T, typename U, typename... Rest>
    struct is_valid_variant_types<T, U, Rest...>
    {
      constexpr static bool value = is_valid_variant_types<T>::value ?
                                    is_valid_variant_types<U, Rest...>::value :
                                    is_valid_variant_types<T, Rest...>::value;
    };

    template<typename T, typename U, typename... Types> struct is_in;
    template<typename T, typename U> struct is_in<T, U>
    {
      constexpr static bool value = std::is_same<T, U>::value;
    };
    template<typename T, typename _1ST, typename... Rest>
    struct is_in
    {
      private:
        constexpr static bool tmp = is_in<T, _1ST>::value;
      public:
        constexpr static bool value = tmp ? tmp : is_in<T, Rest...>::value;
    };

    template<bool ok, typename True, typename False> struct cond
    {
      using type = True;
    };
    template<typename True, typename False> struct cond<false, True, False>
    {
      using type = False;
    };

    template<typename, typename, typename...> struct ctor_type;
    template<typename T, typename U> struct ctor_type<T, U>
    {
      using type = typename cond<std::is_constructible<U, T>::value, U, T>::type;
    };
    template<typename T, typename U, typename... Rest> struct ctor_type
    {
      using type = typename ctor_type<typename ctor_type<T, U>::type, Rest...>::type;
    };

    template<typename T, typename U, typename... Rest> struct is_construct_from
    {
      private:
        using Type = typename ctor_type<T, U, Rest...>::type;
      public:
        constexpr static bool value = is_in<Type, U, Rest...>::value;
    };

    // std::is_assignable is not what I want...
    template<typename Rhs, typename Lhs, bool ok = std::is_pod<typename std::remove_reference<Lhs>::type>::value> struct is_assignable
    {
      private:
        using Raw = typename std::decay<Lhs>::type;
        template<typename V> static std::false_type test(...);
        template<typename V, typename = decltype(std::declval<Raw>().operator=(std::declval<Rhs>()))> static std::true_type test(int);
      public:
        constexpr static bool value = decltype(test<Lhs>(0))::value;
    };

    template<typename T, typename U> struct is_assignable<T, U, true>
    {
      private:
        using RawT = typename std::decay<T>::type;
        using RawU = typename std::decay<U>::type;
      public:
        constexpr static bool value = std::is_same<RawT, RawU>::value;
    };

    template<typename From, typename To, typename... Rest> struct assignable_type;
    template<typename From, typename To> struct assignable_type<From, To>
    {
      private:
        using Rfrom = typename std::decay<From>::type;
        using Rto = typename std::decay<To>::type;
        constexpr static bool value = helper_and<is_assignable<From, To>, std::is_convertible<Rfrom, Rto>>::value;
      public:
        using type = typename cond<value, To, From>::type;
    };
    template<typename From, typename To, typename... Rest> struct assignable_type
    {
      using type = typename assignable_type<typename assignable_type<From, To>::type, Rest...>::type;
    };

    template<typename From, typename To, typename... Rest> struct is_assignable_from
    {
      private:
        using type = typename assignable_type<From, To, Rest...>::type;
      public:
        constexpr static bool value = is_in<type, To, Rest...>::value;
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

    template<typename T, typename U, bool = std::is_pod<T>::value> struct variant_assign
    {
      static void assign(void* data, const void* rhs)
      {
        *static_cast<T*>(data) = *static_cast<const U*>(rhs);
      }

      static void move(void *data, void* rhs)
      {
        assign(data, rhs);
      }
    };
    template<typename T, typename U> struct variant_assign<T, U, false>
    {
      static void assign(void* data, const void* rhs)
      {
        reinterpret_cast<T*>(data)->operator=(*static_cast<const U*>(rhs));
      }

      static void move(void* data, void* rhs)
      {
        reinterpret_cast<T*>(data)->operator=(std::move(*static_cast<U*>(rhs)));
      }
    };
    template<> struct variant_assign<void, void> {};

    template<typename F, typename Arg>
    struct is_type_match
    {
      private:
        template<typename Func> struct extract;

        template<typename R, typename A>
        struct extract<R(A)>
        {
          using ires = R;
          using iarg = A;
        };

        template<typename R, typename A>
        struct extract<R(*)(A)> : public extract<R(A)> {};

        template<typename R, typename A>
        struct extract<std::function<R(A)>> : public extract<R(A)> {};

        template<typename R, typename Class, typename A>
        struct extract<R(Class::*)(A)> : public extract<R(A)> {};

        template<typename R, typename Class, typename A>
        struct extract<R(Class::*)(A) const> : public extract<R(A)> {};

        template<typename R, typename Class, typename A>
        struct extract<R(Class::*)(A) volatile> : public extract<R(A)> {};

        template<typename R, typename Class, typename A>
        struct extract<R(Class::*)(A) const volatile> : public extract<R(A)> {};

        template<typename lambda>
        struct extract : public extract<decltype(&lambda::operator())> {};

        template<typename lambda>
        struct extract<lambda&&> : public extract<decltype(&lambda::operator())> {};
      public:
        using arg = Arg;
        using tmp = extract<F>;
        using iarg = typename tmp::iarg;
        constexpr static bool arg_same = std::is_same<arg, iarg>::value;
    };

    template<bool ok>
    struct variant_call_wrapper
    {
      template<typename Type, typename F> static void invoke(Type& arg, F&& f)
      {
        f(arg);
      }
    };
    template<> struct variant_call_wrapper<false>
    {
      template<typename Type, typename F> static void invoke(Type&, F&&) {}
    };

    struct variant_call
    {
      template<typename Type> static void invoke(Type&) {}
      template<typename Type, typename F> static void invoke(Type& arg, F&& f)
      {
        variant_call_wrapper<is_type_match<decltype(f), Type>::arg_same>::invoke(arg, f);
      }
      template<typename Type, typename Fn, typename... Fns> static void invoke(Type& arg, Fn&& f, Fns&&... fs)
      {
        invoke<Type>(arg, std::forward<Fn>(f));
        invoke<Type>(arg, std::forward<Fns>(fs)...);
      }
    };

    template<typename...> struct variant_helper;
    template<> struct variant_helper<>
    {
      static void clear(int, void*) {}
      static void copy(int, void*, const void*, size_t) {}
      static void move(int, void*, void*, size_t) {}
      template<typename... Fn>
      static void call(int, void*, Fn&&... fs) {}
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
          // don't rely on pseudo-destructor
          if(!std::is_pod<T>::value)
          {
            reinterpret_cast<T*>(data)->~T();
          }
        }
      }

      static void copy(int index, void* lhs, const void* rhs, size_t n)
      {
        if(index > 0)
        {
          index -= 1;
          variant_helper<R...>::copy(index, lhs, rhs, n);
        }
        else if(index == 0)
        {
          if(std::is_pod<T>::value)
          {
            memcpy(lhs, rhs, n);
          }
          else
          {
            static_assert(std::is_copy_constructible<T>::value,
                          "type is not copy constructible");
            new(lhs) T(*static_cast<const T*>(rhs));
          }
        }
      }

      static void move(int index, void* lhs, void* rhs, size_t n)
      {
        if(index > 0)
        {
          index -= 1;
          variant_helper<R...>::move(index, lhs, rhs, n);
        }
        else if(index == 0)
        {
          if(std::is_pod<T>::value)
          {
            memcpy(lhs, rhs, n);
          }
          else
          {
            static_assert(std::is_move_constructible<T>::value,
                          "type is not move constructible");
            new(lhs) T(std::move(*static_cast<T*>(rhs)));
          }
        }
      }

      template<typename... Fn> static void call(int index, void* data, Fn&&... f)
      {
        if(index > 0)
        {
          index -= 1;
          variant_helper<R...>::call(index, data, std::forward<Fn>(f)...);
        }
        else
        {
          variant_call::invoke<T, Fn...>(*static_cast<T*>(data), std::forward<Fn>(f)...);
        }
      }
    };
  }
}

namespace nm
{
  template<typename... Rest>
  class variant
  {
      static_assert(meta::is_valid_variant_types<Rest...>::value, "invalid variant types");
      using helper = meta::variant_helper<Rest...>;
      template<typename T, typename...Args> using and_ = meta::helper_and<T, Args...>;
      template<typename T, typename...Args> using or_ = meta::helper_or<T, Args...>;
      template<typename T> using not_ = meta::helper_not<T>;
    public:
      ~variant()
      {
        clear();
      }

      variant() : type_index_(-1) {}

      template<typename T> variant(T& rhs, typename std::enable_if<
          and_<not_<std::is_const<T>>, not_<std::is_same<T, variant>>,
            meta::is_construct_from<T&, Rest...>>::value>::type* = nullptr)
        : variant()
      {
        assign(rhs);
      }

      template<typename T> variant(const T& rhs, typename std::enable_if<
        and_<not_<std::is_same<T, variant>>,
          meta::is_construct_from<const T&, Rest...>>::value>::type* = nullptr)
        : variant()
      {
        assign(rhs);
      }

      template<typename T> variant(T&& rhs, typename std::enable_if<
        and_<std::is_rvalue_reference<T&&>, not_<std::is_const<T>>,
          not_<std::is_same<T, variant>>,
          meta::is_construct_from<T&&, Rest...>>::value>::type* = nullptr)
        : variant()
      {
        using Type = typename meta::ctor_type<T&&, Rest...>::type;
        new(static_cast<void*>(&data_)) Type(std::move(rhs));
        update_index<Type>();
      }

      variant(variant& rhs)
        : variant()
      {
        copy_construct(rhs);
      }

      variant(const variant& rhs)
        : variant()
      {
        copy_construct(rhs);
      }

      variant(variant&& rhs) noexcept
        : variant()
      {
        move_construct(std::forward<variant>(rhs));
      }

      template<typename T> variant& operator= (const T& rhs)
      {
        using AT = typename meta::assignable_type<const T&, Rest...>::type;
        using Type = typename std::enable_if<meta::is_assignable_from<const T&, Rest...>::value, AT>::type;
        int tmp = meta::index_of_type<AT>::value;
        if(type_index_ != tmp)
        {
          clear();
          new(static_cast<void*>(&data_)) Type(rhs);
        }
        else
        {
          using From = typename std::remove_reference<T>::type;
          meta::variant_assign<Type, From>::assign(&data_, &rhs);
        }
        update_index<Type>();
        return *this;
      }

      template<typename T> variant& operator= (T&& rhs)
      {
        using AT = typename meta::assignable_type<T&&, Rest...>::type;
        int tmp = meta::index_of_type<AT>::value;
        using Type = typename std::enable_if<meta::is_assignable_from<T&&, Rest...>::value, AT>::type;
        bool is_lvalue = std::is_lvalue_reference<decltype(rhs)>::value;
        if(type_index_ != tmp)
        {
          clear();
          if(is_lvalue)
          {
            new(static_cast<void*>(&data_)) Type(rhs);
          }
          else
          {
            new(static_cast<void*>(&data_)) Type(std::move(rhs));
          }
        }
        else
        {
          using From = typename std::remove_reference<T>::type;
          if(is_lvalue)
          {
            meta::variant_assign<Type, From>::assign(&data_, &rhs);
          }
          else
          {
            meta::variant_assign<Type, From>::move(&data_, &rhs);
          }
        }
        update_index<Type>();
        return *this;
      }

      variant& operator= (const variant& rhs)
      {
        if(this == &rhs)
        {
          return *this;
        }
        clear();
        copy_construct(rhs);
        return *this;
      }

      variant& operator= (variant&& rhs)
      {
        if(this == &rhs)
        {
          return *this;
        }
        clear();
        move_construct(std::forward<variant>(rhs));
        return *this;
      }

      template<typename... Args>
      bool operator== (const variant<Args...>& rhs)
      {
        return std::is_same<std::tuple<Rest...>, std::tuple<Args...>>::value;
      }

      template<typename... Args>
      bool operator!= (const variant<Args...> & rhs)
      {
        return !(*this == rhs);
      }

      // what `set` do is clear old value and create new value when type is not const
      template<typename T> void set(T& rhs, typename std::enable_if<meta::is_in<T, Rest...>::value>::type* = nullptr)
      {
        using Type = typename meta::ctor_type<T&, Rest...>::type;
        clear();
        new(static_cast<void*>(&data_)) typename std::enable_if<not_<std::is_const<Type>>::value, Type>::type(rhs);
        update_index<Type>();
      }

      template<typename T> void set(const T& rhs, typename std::enable_if<meta::is_in<T, Rest...>::value>::type* = nullptr)
      {
        using Type = typename meta::ctor_type<T&, Rest...>::type;
        clear();
        new(static_cast<void*>(&data_)) typename std::enable_if<not_<std::is_const<Type>>::value, Type>::type(rhs);
        update_index<Type>();
      }

      template<typename T> void set(T&& rhs, typename std::enable_if<meta::is_in<T, Rest...>::value>::type* = nullptr)
      {
        using Type = typename meta::ctor_type<T&&, Rest...>::type;
        clear();
        new(static_cast<void*>(&data_)) typename std::enable_if<not_<std::is_const<Type>>::value, Type>::type(std::move(rhs));
        update_index<T>();
      }

      template<typename T, typename... Args>
      void emplace(Args&&... args)
      {
        check<T>();
        clear();
        new(static_cast<void*>(&data_)) T(std::forward<Args>(args)...);
        update_index<T>();
      }

      template<typename T> T& get(typename std::enable_if<meta::is_in<T, Rest...>::value>::type* = nullptr)
      {
        if(type_index_ < 0)
          throw std::runtime_error("bad get on an empty variant");
        int tmp = meta::index_of_type<T, Rest...>::value;
        if(type_index_ != tmp)
          throw std::runtime_error("get type mismatch set type");
        return *reinterpret_cast<T*>(&data_);
      }

      template<typename F, typename... Fs> void call(F&& func, Fs&&...funcs)
      {
        helper::call(type_index_, &data_, std::forward<F>(func), std::forward<Fs>(funcs)...);
      }

      void clear()
      {
        helper::clear(type_index_, &data_);
        type_index_ = -1;
      }

      int which() const
      {
        return type_index_;
      }

    private:
      int type_index_;
      constexpr static size_t data_size = meta::max_size_of<sizeof(Rest)...>::value;
      constexpr static size_t align_size = meta::max_size_of<alignof(Rest)...>::value;
      typename std::aligned_storage<data_size, align_size>::type data_;

      template<typename T> void check()
      {
        static_assert(meta::is_in<T, Rest...>::value, "invalid type");
      }

      template<typename T> void update_index()
      {
        type_index_ = meta::index_of_type<T, Rest...>::value;
      }

      template<typename T> void assign(const T& rhs)
      {
        using Type = typename meta::ctor_type<const T&, Rest...>::type;
        new(static_cast<void*>(&data_)) Type(rhs);
        update_index<Type>();
      }

      void copy_construct(const variant& rhs)
      {
        helper::copy(rhs.type_index_, &data_, &rhs.data_, data_size);
        type_index_ = rhs.type_index_;
      }

      void move_construct(variant&& rhs)
      {
        helper::move(rhs.type_index_, &data_, &rhs.data_, data_size);
        type_index_ = rhs.type_index_;
        rhs.clear(); // make it invalid
      }
  };
}

#endif // VARIANT_H__
