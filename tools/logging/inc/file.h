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
  class File
  {
    public:
      File(const char* path);
      ~File();
      void append(const char* line, int len);
      int flush();
      int rest() const;
      size_t index() const;
      bool avail() const;
    private:
      enum { BUFFER_SIZE = 64 * 1024 };
      char buf_[BUFFER_SIZE];
      FILE* fp_;
      size_t index_;
  };
  class FileCtl
  {
    public:
      FileCtl(const std::string& path,
          size_t size_limit,
          time_t duration,
          bool thread_safe,
          int flush_interval,
          int check_interval);
      ~FileCtl() { file_.reset(nullptr); }
      template<int N>
      void append(const char (&line)[N]);
      void append(const char* line, const int len);
      void flush();
    private:
      std::string root_;
      size_t size_limit_;
      time_t duration_;
      bool thread_safe_;
      int flush_interval_;
      int check_interval_;
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

#endif
