/*********************************************************
          File Name:logbase.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 10:27:39 AM CST
**********************************************************/

#ifndef LOG_BASE_H_
#define LOG_BASE_H_

namespace nm
{
  struct uniq
  {
    uniq(){}
    ~uniq(){}
    uniq(const uniq&) = delete;
    uniq(uniq&&) = delete;
    uniq& operator= (const uniq&) = delete;
    uniq& operator= (uniq&&) = delete;
  };
  enum LogLevel
  {
    INFO = 0,
    DEBUG = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
  };
  int gettid();
}

#endif
