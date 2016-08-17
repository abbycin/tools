/*********************************************************
          File Name:logging.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 03:41:03 PM CST
**********************************************************/

#ifndef LOGGING_H_
#define LOGGING_H_

#include "meta/logbase.h"
#include "meta/file.h"
#include "meta/stream_wrapper.h"
#include <cassert>

namespace nm
{
  // Thread-safe by default
  class Logging : meta::uniq
  {
    private:
      class Logger
      {
        public:
          Logger(const std::string& path,
              size_t size_limit,
              time_t duration,
              bool thread_safe,
              int flush_interval,
              int check_interval);
          template<int N>
          Logger& operator<< (const char (&arr)[N])
          {
            stream_.append(arr, N - 1);
            return *this;
          }
          template<typename T>
          Logger& operator<< (const T x)
          {
            stream_ << x;
            return *this;
          }
          Logger& operator() (const char*, int, LogLevel);
          Logger& operator() (const char*, int, LogLevel,
              const char*, bool is_abort = false);
          void flush()
          {
            stream_.flush();
          }
        private:
          int tm_len_;
          meta::TimeFmt tm_time_;
          meta::FileStream stream_;
      };
      static std::unique_ptr<Logger> logger_;
     Logging();
      ~Logging();
    public:
      static void create_instance(const std::string& path,
          size_t size_limit = 0,
          time_t duration = 60 * 60 * 24,
          bool thread_safe = true,
          int flush_interval = 3,
          int check_interval = 1024)
      {
        logger_.reset(new Logger(path, size_limit, duration,
          thread_safe, flush_interval, check_interval));
      }
      static Logger& instance()
      {
        assert(logger_);
        return (*logger_.get());
      }
  };
#define LOG_INFO Logging::instance()(__FILE__, __LINE__, nm::LogLevel::INFO)
#define LOG_WARN Logging::instance()(__FILE__, __LINE__, nm::LogLevel::WARNING)
#define LOG_DEBUG Logging::instance()(__FILE__, __LINE__, nm::LogLevel::DEBUG, __func__)
#define LOG_ERR Logging::instance()(__FILE__, __LINE__, nm::LogLevel::ERROR)
#define LOG_FATAL Logging::instance()(__FILE__, __LINE__, nm::LogLevel::FATAL, __func__, true)
#define LOG_FLUSH Logging::instance().flush()
}

#endif
