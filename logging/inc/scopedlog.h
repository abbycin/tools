/*********************************************************
          File Name:scopedlog.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 10:29:07 AM CST
**********************************************************/

#ifndef SCOPED_LOG_H_
#define SCOPED_LOG_H_

#include "meta/logbase.h"
#include "meta/stream_wrapper.h"
#include "meta/timefmt.h"

namespace nm
{
  // NOT thread-safe, users should use Macros instead
  class ScopedLog : meta::uniq
  {
    public:
      ScopedLog(const meta::Path&, int line, LogLevel);
      ScopedLog(const meta::Path&, int line, LogLevel level, const char* func);
      ~ScopedLog();
      meta::BufferStream& stream();
      static void set_appender(std::function<void(const char*, const size_t)>);
      static void set_flush(std::function<void()>);
    private:
      class wrapper
      {
        public:
          wrapper(const meta::Path&, int line, LogLevel level);
          wrapper(const meta::Path&, int line, LogLevel level, const char* func);
          ~wrapper();
          meta::BufferStream& stream();
          const char* data() const;
          size_t size() const;
        private:
          int tm_len_;
          meta::TimeFmt tm_time_;
          meta::BufferStream stream_;
      };
      LogLevel level_;
      wrapper logger_;
  };

#define SCOPED_INFO ScopedLog(__FILE__, __LINE__, nm::LogLevel::INFO).stream()
#define SCOPED_DEBUG ScopedLog(__FILE__, __LINE__, nm::LogLevel::DEBUG, __func__).stream()
#define SCOPED_WARN ScopedLog(__FILE__, __LINE__, nm::LogLevel::WARNING).stream()
#define SCOPED_ERR ScopedLog(__FILE__, __LINE__, nm::LogLevel::ERROR).stream()
#define SCOPED_FATAL ScopedLog(__FILE__, __LINE__, nm::LogLevel::FATAL, __func__).stream()
}

#endif
