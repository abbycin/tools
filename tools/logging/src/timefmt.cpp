/*********************************************************
          File Name:timefmt.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 01:06:25 PM CST
**********************************************************/

#include "timefmt.h"

namespace nm
{
  TimeFmt TimeFmt::now()
  {
    struct timeval tv;
    if(gettimeofday(&tv, NULL) == -1)
      return TimeFmt(0, 0);
    return TimeFmt(tv.tv_sec, tv.tv_usec);
  }
  std::string TimeFmt::to_string()
  {
    char buf[25] = {0};
    int len = sprintf(buf, "%ld.%06ld", second_,microsecond_);
    return std::string(buf, len);
  }
  std::string TimeFmt::format(bool show_micro)
  {
    char buf[25] = {0};
    struct tm tm_time;
    localtime_r(&second_, &tm_time);  // we don't use gmtime.
    int len = 0;
    if(show_micro)
    {
      len = sprintf(buf, "%4d%02d%02d_%02d%02d%02d.%04ldu.",
          tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
          tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microsecond_);
    }
    else
    {
      len = sprintf(buf, "%4d%02d%02d_%02d%02d%02d.",
          tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
          tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return std::string(buf, len);
  }
}
