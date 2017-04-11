/*********************************************************
          File Name: logging.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 08 Apr 2017 07:05:39 PM CST
**********************************************************/

#ifndef LOGGING_H_
#define LOGGING_H_

#include <string>
#include <sstream>
#include <functional>

namespace nm
{
  class Stream; 
  class Logger
  {
    private:
      class Wrapper
      {
        public:
          Wrapper(std::stringstream& s)
            : ss_(s)
          {}

          template<typename T> Wrapper& operator<< (const T& data)
          {
            ss_ << data;
            return *this;
          }

        private:
          std::stringstream& ss_;
      };
    public:
      enum Level { INFO = 0, WARNING, DEBUG, ERR, FATAL };
      // default interval is 24hr in milliseconds
      static bool create(const std::string& path = "",
                           const std::string& prefix = "",
                           long interval = 1000 * 60 * 60 * 24);
      // throw 'invalid receiver' when no Logger::create be called.
      Logger(const char*, long, const char*, Level);

      Logger(const Logger&) = delete;
      Logger(Logger&&);
      ~Logger();

      Wrapper stream();

    private:
      Stream* stream_;
  };

#define LOG_INFO nm::Logger(__FILE__, __LINE__, nullptr, nm::Logger::INFO).stream()
#define LOG_WARN nm::Logger(__FILE__, __LINE__, nullptr, nm::Logger::WARNING).stream()
#define LOG_DEBUG nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::DEBUG).stream()
#define LOG_ERR nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::ERR).stream()
#define LOG_FATAL nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::FATAL).stream()
}

#endif // LOGGING_H_
