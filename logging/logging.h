/*********************************************************
          File Name: logging.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 08 Apr 2017 07:05:39 PM CST
**********************************************************/

#ifndef LOGGING_H_
#define LOGGING_H_

#include <sstream>
#include <functional>

namespace nm
{
  class Logger
  {
    public:
      struct Dummy
      {
        template<typename T> Dummy& operator<< (const T&) { return *this; }
      };
      enum Level { INFO = 0, WARNING, DEBUG, ERR, FATAL };
      // default interval is 24hr in milliseconds, minimum duration is 1s.
      static bool create_async(const char* path = "",
                               const char* prefix = "",
                               long interval = 60 * 60 * 24);
      static bool create_sync(const char* path = "",
                              const char* prefix = "",
                              long interval = 60 * 60 * 24);
      Logger(const char*, long, const char*, Level);

      template<typename T> Logger& operator<< (const T& data)
      {
        ss_ << data;
        return *this;
      }

      Logger(const Logger&) = delete;
      Logger(Logger&&) = delete;
      Logger& operator= (const Logger&) = delete;
      Logger& operator= (Logger&&) = delete;
      ~Logger();

    private:
      thread_local static std::stringstream ss_;
      static std::function<void(std::stringstream&)> cb_;
  };

#ifndef NOLOG
#define LOG_DISABLED false
#define LOG_INFO nm::Logger(__FILE__, __LINE__, nullptr, nm::Logger::INFO)
#define LOG_WARN nm::Logger(__FILE__, __LINE__, nullptr, nm::Logger::WARNING)
#define LOG_DEBUG nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::DEBUG)
#define LOG_ERR nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::ERR)
#define LOG_FATAL nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::FATAL)
#else
#define LOG_DISABLED true
#define LOG_INFO nm::Logger::Dummy()
#define LOG_WARN LOG_INFO
#define LOG_DEBUG LOG_INFO
#define LOG_ERR LOG_INFO
#define LOG_FATAL LOG_INFO
#endif
}

#endif // LOGGING_H_
