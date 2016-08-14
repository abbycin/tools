/*********************************************************
          File Name:scopedlog.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 10:29:07 AM CST
**********************************************************/

#ifndef SCOPED_LOG_H_
#define SCOPED_LOG_H_

#include "logbase.h"
#include "stream_wrapper.h"
#include "timefmt.h"

namespace nm
{
  class ScopedLog : uniq
  {
    public:
      ScopedLog(const char* file, int line, LogLevel);
      ScopedLog(const char* file, int line, LogLevel level, const char* func);
      ~ScopedLog();
      StreamWrapper& stream();
      typedef void (*out)(const char*, int);
      typedef void (*flush_out)();
      static void set_appender(out);
      static void set_flush(flush_out);
    private:
      class wrapper
      {
        public:
          wrapper(const char* file, int line, LogLevel level);
          wrapper(const char* file, int line, LogLevel level, const char* func);
          ~wrapper();
          StreamWrapper& stream();
          const char* data() const;
          int size() const;
        private:
          const char** msg = LOGMSG;
          StreamWrapper stream_;
      };
      LogLevel level_;
      wrapper logger_;
  };

#define LOGINFO ScopedLog(__FILE__, __LINE__, nm::LogLevel::INFO).stream()
#define LOGDEBUG ScopedLog(__FILE__, __LINE__, nm::LogLevel::DEBUG, __func__).stream()
#define LOGWARN ScopedLog(__FILE__, __LINE__, nm::LogLevel::WARNING).stream()
#define LOGERR ScopedLog(__FILE__, __LINE__, nm::LogLevel::ERROR).stream()
#define LOGFATAL ScopedLog(__FILE__, __LINE__, nm::LogLevel::FATAL, __func__).stream()
}

#endif
