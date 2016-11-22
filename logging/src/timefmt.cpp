/*********************************************************
          File Name:timefmt.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 01:06:25 PM CST
**********************************************************/

#include "meta/timefmt.h"

namespace nm
{
  namespace meta
  {
    thread_local static char __buf[25] = {0};
    TimeFmt TimeFmt::now()
    {
      struct timeval tv;
      if(gettimeofday(&tv, NULL) == -1)
        return TimeFmt(0, 0);
      return TimeFmt(tv.tv_sec, tv.tv_usec);
    }
    std::string TimeFmt::to_string()
    {
      int len = sprintf(__buf, "%ld.%06ld", second_,microsecond_);
      return std::string(__buf, len);
    }
    std::string TimeFmt::format(bool show_micro)
    {
      int len = format(__buf, 32);
      return std::string(__buf, len);
    }
    int TimeFmt::format(char buf[], int size, bool show_micro)
    {
      localtime_r(&second_, &tm_time_);  // we don't use gmtime.
      int len = 0;
      if(show_micro)
      {
        len = snprintf(buf, size, "%4d%02d%02d_%02d%02d%02d.%06ldu.",
                       tm_time_.tm_year + 1900, tm_time_.tm_mon + 1, tm_time_.tm_mday,
                       tm_time_.tm_hour, tm_time_.tm_min, tm_time_.tm_sec, microsecond_);
      }
      else
      {
        len = snprintf(buf, size, "%4d%02d%02d_%02d%02d%02d.",
                       tm_time_.tm_year + 1900, tm_time_.tm_mon + 1, tm_time_.tm_mday,
                       tm_time_.tm_hour, tm_time_.tm_min, tm_time_.tm_sec);
      }
      return len;
    }
    TimeFmt& TimeFmt::update()
    {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      second_ = tv.tv_sec;
      microsecond_ = tv.tv_usec;
      return *this;
    }
  }
}
