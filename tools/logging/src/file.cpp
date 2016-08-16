/*********************************************************
          File Name:file.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 11:23:51 AM CST
**********************************************************/

#include "file.h"
#include <unistd.h>
#include <cassert>
#include <cstdio>

namespace nm
{
  File::File(const char* path)
    : fp_(::fopen(path, "a")), index_(0)
  {
    assert(fp_ != nullptr);
    ::setbuffer(fp_, buf_, BUFFER_SIZE); // use our own buffer
  }
  File::~File()
  {
    ::fclose(fp_);
  }
  void File::append(const char* line, int len)
  {
    int n = ::fwrite_unlocked(line, 1, len, fp_);
    int rest = len - n;
    while(rest > 0)
    {
      int tmp = ::fwrite_unlocked(line + n, 1, rest, fp_);
      if(tmp == 0)
      {
        if(::ferror(fp_))
          fprintf(stderr, "File::append() faild ");
        else
          break;
      }
      n += tmp;
      rest -= tmp;
    }
    index_ += len;
  }
  int File::flush()
  {
    return ::fflush_unlocked(fp_);
  }
  int File::rest() const
  {
    return BUFFER_SIZE - index_;
  }
  size_t File::index() const
  {
    return index_;
  }
  bool File::avail() const
  {
    return !(BUFFER_SIZE == index_);
  }
  FileCtl::FileCtl(const std::string& path, size_t size_limit, time_t duration,
      bool thread_safe, int flush_interval, int check_interval)
    : root_(path), size_limit_(size_limit), duration_(duration),
    thread_safe_(thread_safe), flush_interval_(flush_interval),
    check_interval_(check_interval), count_(0),
    last_flush_(0),last_roll_(0)
  {
    // user must at least ensure the `path` is a valid directory
    assert(root_.find("/") != root_.npos);
    roll_();
  }
  template<int N>
  void FileCtl::append(const char (&line)[N])
  {
    this->append(line, N - 1);
  }
  void FileCtl::append(const char* line, const int len)
  {
    if(thread_safe_)
    {
      std::lock_guard<std::mutex> l(mtx);
      append_(line, len);
    }
    else
    {
      append_(line, len);
    }
  }
  void FileCtl::flush()
  {
    if(thread_safe_)
    {
      std::lock_guard<std::mutex> l(mtx);
      file_->flush();
    }
    else
      file_->flush();
  }
  bool FileCtl::roll_()
  {
    std::string path = get_newname_();
    last_roll_ = tm_cache_.sec();
    last_flush_ = tm_cache_.sec();
    file_.reset(new File(path.c_str()));
    return true;
  }
  // format: argv[0]_yyyymmdd_HHMMSS.nnnnnnZ.pid.log
  std::string FileCtl::get_newname_()
  {
    tm_cache_ = std::move(TimeFmt::now());
    std::string tmp = root_ + tm_cache_.format();
    char pidbuf[10] = {0};
    int len = sprintf(pidbuf, "%d", getpid());
    tmp.append(pidbuf, len);
    return tmp.append(".log");
  }
  void FileCtl::append_(const char* line, const int len)
  {
    file_->append(line, len);
    if(size_limit_ > 0 && file_->index() > size_limit_)
      roll_();
    else
    {
      ++count_;
      if(count_ > check_interval_)
      {
        count_ = 0;
        time_t now = TimeFmt::now().sec();
        if(now - last_roll_ >= duration_)
          roll_();
        else if(now - last_flush_ > flush_interval_)
        {
          last_flush_ = now;
          this->flush();
        }
      }
    }
  }
}
