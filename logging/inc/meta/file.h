/*********************************************************
          File Name:file.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 11:10:46 AM CST
**********************************************************/

#ifndef FILE_H_
#define FILE_H_

#include "timefmt.h"
#include <memory>
#include <mutex>
#include <string>

namespace nm
{
  namespace meta
  {
    // Not thread-safe
    class File;
    // Thread-safe or not, its depends
    class FileCtl
    {
      public:
        FileCtl(const std::string& path,
                size_t size_limit,
                time_t duration,
                bool thread_safe,
                int flush_interval,
                int check_interval);
        ~FileCtl();
        void append(const char* line, const size_t len);
        void flush();
      private:
        const std::string root_;
        const std::string current_log_;
        const size_t size_limit_;
        const time_t duration_;
        const bool thread_safe_;
        const int flush_interval_;
        const int check_interval_;
        int count_;
        time_t last_flush_;
        time_t last_roll_;
        TimeFmt tm_cache_;  // update when create new log file
        std::unique_ptr<File> file_;
        std::mutex mtx;
        bool roll_();
        std::string get_newname_();
        void append_(const char* line, const int len);
    };
  }
}

#endif
