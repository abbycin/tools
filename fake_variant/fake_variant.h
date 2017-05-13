/*********************************************************
          File Name: fakevariant.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Mon 21 Nov 2016 09:32:37 PM CST
**********************************************************/

#ifndef VARIANT_H_
#define VARIANT_H_

#include <functional>

namespace nm
{
  namespace meta
  {
    struct Nil {};

    template<typename T, typename U>
    struct TypeList
    {
      using Head = T;
      using Tail = U;
    };

    template<typename... Args> struct GenList;
    template<> struct GenList<Nil>
    {
      using type = Nil; 
    };
    template<typename T>
    struct GenList<T>
    {
      using type = TypeList<T, Nil>;
    };
    template<typename... Args>
    struct GenList<Nil, Args...> // ignore `Nil` in Args
    {
      using type = typename GenList<Args...>::type;
    };
    template<typename T, typename... Args>
    struct GenList<T, Args...>
    {
      using type = TypeList<T, typename GenList<Args...>::type>;
    };

    template<typename, typename> struct Remove;
    template<typename T>
    struct Remove<Nil, T>
    {
      using type = Nil;
    };
    template<typename Tail, typename T>
    struct Remove<TypeList<T, Tail>, T>
    {
      using type = Tail;
    };
    template<typename Head, typename Tail, typename T>
    struct Remove<TypeList<Head, Tail>, T>
    {
      using type = TypeList<Head, typename Remove<Tail, T>::type>;
    };

    template<typename> struct Unique;
    template<> struct Unique<Nil>
    {
      using type = Nil;
    };
    template<typename Head, typename Tail>
    struct Unique<TypeList<Head, Tail>>
    {
      private:
        using tmp = typename Unique<Tail>::type;
        using tail = typename Remove<tmp, Head>::type;
      public:
        using type = TypeList<Head, tail>;
    };

    template<typename TL, template<typename> class Class> class GenClasses;

    template<typename T1, typename T2, template<typename> class Class>
    class GenClasses<TypeList<T1, T2>, Class> : public GenClasses<T1, Class>,
          public GenClasses<T2, Class>
    {};

    template<typename T, template<typename> class Class>
    class GenClasses : public Class<T>
    {};

    template<template<typename> class Class>
    class GenClasses<Nil, Class>
    {};
    
  }

  template<typename U, typename... Args>
  class FakeVariant
  {
    public:
      template<typename T> bool set(const T& value)
      {
        return data_.invisible<T>::set(value);
      }
      template<typename T, typename... Paras>
      bool set(Paras&&... paras)
      {
        return data_.invisible<T>::set(std::forward<Paras>(paras)...);
      }
      template<typename T> const T& get() const
      {
        return data_.invisible<T>::get();
      }
      template<typename T> bool valid() const
      {
        return data_.invisible<T>::valid();
      }
    private:
      template<typename T>
      class invisible
      {
        public:
          invisible()
            : value_{nullptr}
          {}
          ~invisible()
          {
            clear();
          }
          template<typename... Paras>
          bool set(Paras&&... paras)
          {
            bool res = valid();
            clear();
            value_ = new T(std::forward<Paras>(paras)...);
            return !res;
          }
          const T& get() const
          {
            return *value_;
          }
          bool valid() const
          {
            return value_ != nullptr;
          }
        private:
          T* value_;
          void clear()
          {
            if(valid())
            {
              delete value_;
              value_ = nullptr;
            }
          }
      };
      meta::GenClasses<typename meta::Unique<typename meta::GenList<U, Args...>
        ::type>::type, invisible> data_;
  };
}

#endif
