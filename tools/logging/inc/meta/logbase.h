/*********************************************************
          File Name:logbase.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 10:27:39 AM CST
**********************************************************/

#ifndef LOG_BASE_H_
#define LOG_BASE_H_

#include <cstring>
#include <cstdio>

namespace nm
{
  namespace meta
  {
    struct uniq
    {
      uniq(){}
      virtual ~uniq(){}
      uniq(const uniq&) = delete;
      uniq(uniq&&) = delete;
      uniq& operator= (const uniq&) = delete;
      uniq& operator= (uniq&&) = delete;
    };
    int gettid();
    class Path
    {
      public:
        template<unsigned int N>
        Path(const char (&arr)[N])
          : path_(arr), base_(nullptr), len_(N - 1)
        {
          base_ = strrchr(arr, '/');
          if(base_)
            base_ = base_ + 1;
        }
        const char* full_path() const
        {
          return path_;
        }
        const char* base_path() const
        {
          if(base_)
            return base_;
          return path_;
        }
        size_t full_len() const
        {
          return len_;
        }
        size_t base_len() const
        {
          if(base_)
            return (path_ + len_ - base_);
          return len_;
        }
      private:
        const char* path_;
        const char* base_;
        size_t len_;
    };
  }
  enum LogLevel
  {
    INFO = 0,
    DEBUG = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
  };
}

#endif
