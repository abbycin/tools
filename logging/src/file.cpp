/*********************************************************
          File Name:file.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 11:23:51 AM CST
**********************************************************/

#include "meta/file.h"
#include <unistd.h>
#include <cassert>
#include <cstdio>

namespace nm
{
  namespace meta
  {
    class File
    {
      public:
        File(const char* path);
        ~File();
        void append(const char* line, size_t len);
        int flush();
        size_t write_bytes() const;
      private:
        enum { BUFFER_SIZE = 64 * 1024 };
        char buf_[BUFFER_SIZE];
        FILE* fp_;
        size_t write_bytes_;
    };

    File::File(const char* path)
      : fp_(::fopen(path, "a")), write_bytes_(0)
    {
      assert(fp_ != nullptr);
      ::setbuffer(fp_, buf_, BUFFER_SIZE); // use our own buffer
    }
    File::~File()
    {
      ::fclose(fp_);
    }
    void File::append(const char* line, size_t len)
    {
      size_t n = ::fwrite_unlocked(line, 1, len, fp_);
      size_t rest = len - n;
      while(rest > 0)
      {
        size_t tmp = ::fwrite_unlocked(line + n, 1, rest, fp_);
        if(tmp == 0)
        {
          if(::ferror(fp_))
            fprintf(stderr, "File::append() failed ");
          else
            break;
        }
        n += tmp;
        rest -= tmp;
      }
      write_bytes_ += len;
    }
    int File::flush()
    {
      return ::fflush_unlocked(fp_);
    }
    size_t File::write_bytes() const
    {
      return write_bytes_;
    }
    FileCtl::FileCtl(const std::string& path, size_t size_limit, time_t duration,
                     bool thread_safe, int flush_interval, int check_interval)
      : root_(path), current_log_(root_ + "current.log"),
        size_limit_(size_limit), duration_(duration),
        thread_safe_(thread_safe), flush_interval_(flush_interval),
        check_interval_(check_interval),
        count_(0), last_flush_(0),last_roll_(0)
    {
      assert(root_.size() > 0);
      assert(root_.find_last_of("/") == root_.size() - 1);
      roll_();
    }
    FileCtl::~FileCtl() { file_.reset(nullptr); }
    void FileCtl::append(const char* line, const size_t len)
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
      unlink(current_log_.c_str());
      symlink(path.c_str(), current_log_.c_str());
      last_roll_ = tm_cache_.sec();
      last_flush_ = tm_cache_.sec();
      file_.reset(new File(path.c_str()));
      return true;
    }
    // format: argv[0]_yyyymmdd_HHMMSS.nnnnnnZ.pid.log
    std::string FileCtl::get_newname_()
    {
      tm_cache_.update();
      std::string tmp = root_ + tm_cache_.format();
      char pidbuf[10] = {0};
      int len = sprintf(pidbuf, "%d", getpid());
      tmp.append(pidbuf, len);
      return tmp.append(".log");
    }
    void FileCtl::append_(const char* line, const int len)
    {
      file_->append(line, len);
      if(size_limit_ > 0 && file_->write_bytes() > size_limit_)
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
}
