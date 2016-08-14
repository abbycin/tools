/*********************************************************
          File Name:timefmt.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 10:18:54 AM CST
**********************************************************/

#ifndef TIME_FMT_H_
#define TIME_FMT_H_

#include <ctime>
#include <string>
#include <sys/time.h> // gettimeofday()

namespace nm
{
  class TimeFmt
  {
    public:
      static TimeFmt now();
      TimeFmt(): second_(0), microsecond_(0){}
      TimeFmt(time_t second, int micro)
      {
        second_ = second;
        microsecond_ = micro;
      }
      TimeFmt(const TimeFmt&& t)
      {
        second_ = t.second_;
        microsecond_ = t.microsecond_;
      }
      TimeFmt& operator= (TimeFmt&& t)
      {
        second_ = t.second_;
        microsecond_ = t.microsecond_;
        return *this;
      }
      std::string to_string();
      std::string format(bool show_micro = true);
      time_t sec() const
      {
        return second_;
      }
      long msec() const
      {
        return microsecond_;
      }
      bool valid() const
      {
        return second_ != 0;
      }
    private:
      TimeFmt(const TimeFmt&) = delete;
      TimeFmt& operator= (const TimeFmt&) = delete;
      time_t second_;
      long microsecond_;
  };
}

#endif
