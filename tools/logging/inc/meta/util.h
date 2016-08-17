/*********************************************************
          File Name:util.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Mon 15 Aug 2016 03:54:29 PM CST
**********************************************************/

#ifndef UTIL_H_
#define UTIL_H_

#include <limits>
#include <cstring>
#include <cinttypes>

namespace nm
{
  namespace meta
  {
    struct NullType {};
    template<typename T, typename U>
    struct TypeList
    {
      typedef T Head;
      typedef U Tail;
    };
#define TL1(T1) TypeList<T1, NullType>
#define TL2(T1, T2) TypeList<T1, TL1(T2)>
    template<typename> struct Max;
    template<> struct Max<NullType>
    {
      enum { integer = 0, decimal = 0, total = 0 };
    };
    template<typename Head, typename Tail>
    struct Max<TypeList<Head, Tail>>
    {
      private:
        enum
        {
          cur_int = std::numeric_limits<Head>::digits,
          nxt_int = Max<Tail>::integer,
          cur_dec = std::numeric_limits<Head>::digits10,
          nxt_dec = Max<Tail>::decimal
        };
      public:
        enum
        {
          integer = cur_int > nxt_int ? cur_int : nxt_int,
          decimal = cur_dec > nxt_dec ? cur_dec : nxt_dec,
          total = integer + decimal
        };
    };

    thread_local static char __buf[Max<TL2(long, long double)>::total + 1] = {0};

    template<typename T>
    int convert(const char* fmt, const T x)
    {
      return snprintf(__buf, sizeof __buf, fmt, x);
    }
    template<typename Obj> void append(Obj* obj, const char* s)
    {
      obj->append(s, strlen(s));
    }
    template<typename Obj> void append(Obj* obj, const bool b)
    {
      if(b)
        obj->append("true", 4);
      else
        obj->append("false", 5);
    }
    template<typename Obj> void append(Obj* obj, const char c)
    {
      obj->append(&c, 1);
    }
    template<typename Obj> int append(Obj* obj, const int n)
    {
      int len = convert("%d", n);
      obj->append(__buf, len);
      return len;
    }
    template<typename Obj> int append(Obj* obj, const unsigned int n)
    {
      int len = convert("%u", n);
      obj->append(__buf, len);
      return len;
    }
    template<typename Obj> int append(Obj* obj, const long n)
    {
      int len = convert("%ld", n);
      obj->append(__buf, len);
      return len;
    }
    template<typename Obj> int append(Obj* obj, const unsigned long n)
    {
      int len = convert("%lu", n);
      obj->append(__buf, len);
      return len;
    }
    template<typename Obj> int append(Obj* obj, const float n)
    {
      int len = convert("%.6f", n);
      obj->append(__buf, len);
      return len;
    }
    template<typename Obj> int append(Obj* obj, const double n)
    {
      int len = convert("%.10g", n);
      obj->append(__buf, len);
      return len;
    }
    template<typename Obj> int append(Obj* obj, const void* p)
    {
      uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
      int len = convert("0x%lx", ptr);
      obj->append(__buf, len);
      return len;
    }
  }
}

#endif
