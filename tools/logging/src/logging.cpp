/*********************************************************
          File Name:logging.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 03:51:01 PM CST
**********************************************************/

#include "logging.h"
#include <stdlib.h>

namespace nm
{
  std::unique_ptr<Logging::Logger> Logging::logger_ = nullptr;
  Logging::Logger::Logger(const std::string& path,
      size_t size_limit,
      time_t duration,
      bool thread_safe,
      int flush_interval,
      int check_interval)
    : file_(path.c_str(), size_limit, duration,
        thread_safe, flush_interval, check_interval), stream_()
  {

  }
  Logging::Logger& Logging::Logger::operator() (const char* file, int line,
      LogLevel level)
  {
      stream_ << TimeFmt::now().format() << gettid()
        << LOGMSG[level] << file << " " << line << " => ";
      file_.append(stream_.buffer().data(), stream_.buffer().index());
      stream_.reset();
      return *this;
  }
  Logging::Logger& Logging::Logger::operator() (const char* file, int line,
      LogLevel level, bool is_abort)
  {
    (*this)(file, line, level);
    if(is_abort)
    {
      file_.flush();
      abort();
    }
    return *this;
  }
  Logging::Logger& Logging::Logger::operator() (const char* file, int line,
      LogLevel level, const char* func)
  {
      stream_ << TimeFmt::now().format() << gettid()
        << LOGMSG[level] << file << " " << line
        << " `" << func << "` => ";
      file_.append(stream_.buffer().data(), stream_.buffer().index());
      stream_.reset();
      return *this;
  }
}
