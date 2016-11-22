/*********************************************************
          File Name:util.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Mon 21 Nov 2016 08:57:04 PM CST
**********************************************************/

#ifndef UTIL_H_
#define UTIL_H_

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
}

#endif
