/*********************************************************
          File Name: logging.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 08 Apr 2017 11:13:12 PM CST
**********************************************************/

#include "logging.h"
#include "mpsc.h"
#include <functional>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <cerrno>
#include <cstdlib>
#include <sys/time.h>
#include <syscall.h>

namespace nm
{
  inline int get_tid()
  {
    return static_cast<int>(syscall(SYS_gettid));
  }

  static const char* MSG[] = {
    " [INFO]  ",
    " [WARN]  ",
    " [DEBUG] ",
    " [ERROR] ",
    " [FATAL] "
  };

  class Stream
  {
    public:
      Stream(Sender<std::string>& s)
        : sender_(s)
      {}

      ~Stream() = default;

      void finish(std::stringstream& ss)
      {
        ss << "\n";
        sender_.send(ss.str());
      }

    private:
      Sender<std::string>& sender_;
  };

  class FileLog
  {
    public:
      FileLog(Receiver<std::string>& rx, const std::string& path,
              const std::string& prefix, long interval)
        : rcv_(rx), path_(path), interval_(interval),
          count_(COUNT_DOWN), time_cache_(0), last_roll_(0), fp_(nullptr)
      {
        if(path_.empty())
        {
          path_ = "./";
        }
        else
        {
          auto pos = path_.find_last_of("/");
          if(pos == path_.npos || pos +1 != path_.size())
            path_ += "/";
        }
        current_log_ = path_ + "current.log";
        if(!prefix.empty())
          path_ += prefix + "_";
      }

      ~FileLog()
      {
        if(fp_)
        {
          fflush_unlocked(fp_);
          fclose(fp_);
        }
      }

      bool init()
      {
        errno = 0;
        time_cache_ = now();
        last_roll_ = time_cache_;
        update_time();
        fp_ = fopen(new_name().c_str(), "a+");
        if(fp_ == nullptr)
        {
          fprintf(stderr, "open '%s': %s\n", name_cache_.c_str(), strerror(errno));
          return false;
        }
        this->link();
        return true;
      }

      void write()
      {
        while(rcv_.is_working())
        {
          rcv_.recv(data_);
          roll();
        }
        while(rcv_.try_recv(data_))
          roll();
      }
    private:
      enum { COUNT_DOWN = 1024 };
      Receiver<std::string>& rcv_;
      std::string path_;
      const long interval_;
      int count_;
      long time_cache_;
      long last_roll_;
      FILE* fp_;
      std::string data_;
      std::string name_cache_;
      std::string current_log_;
      char time_buf_[25];
      char pid_buf_[10];
      timeval tv_;
      tm tm_;

      void link()
      {
        unlink(current_log_.c_str());
        symlink(new_name().c_str(), current_log_.c_str());
      }

      void update_time()
      {
        localtime_r(&tv_.tv_sec, &tm_);
        snprintf(time_buf_, sizeof(time_buf_), "%04d%02d%02d_%02d:%02d:%02d.%06ld",
                 tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday,
                 tm_.tm_hour, tm_.tm_min, tm_.tm_sec, tv_.tv_usec);

      }

      void update_micro()
      {
        snprintf(time_buf_ + 18, 7, "%06ld", tv_.tv_usec);
      }

      // milliseconds
      long now()
      {
        gettimeofday(&tv_, nullptr);
        return tv_.tv_sec;
      }

      std::string& new_name()
      {
        name_cache_.clear();
        memset(pid_buf_, 0, sizeof(pid_buf_));
        sprintf(pid_buf_, "%d", getpid());
        name_cache_ = path_ + time_buf_;
        name_cache_.append("_").append(pid_buf_).append(".log");
        return name_cache_;
      }

      void roll()
      {
        size_t rest = data_.size();
        if(rest == 0)
          return;
        size_t write_bytes = 0;
        const char* data = data_.data();
        now();
        if(tv_.tv_sec > time_cache_)
        {
          time_cache_ = tv_.tv_sec;
          update_time();
        }
        else
        {
          update_micro();
        }
        fwrite_unlocked(time_buf_, 1, sizeof(time_buf_) - 1, fp_);
        while(rest != 0)
        {
          write_bytes = fwrite_unlocked(data, 1, rest, fp_);
          data += write_bytes;
          rest -= write_bytes;
        }
        data_.clear();
        if(count_-- >= 0)
          return;
        fflush_unlocked(fp_);
        count_ = COUNT_DOWN;
        if(interval_ <= 0)
          return;
        if(time_cache_ - last_roll_ > interval_)
        {
          last_roll_ = time_cache_;
          fflush(fp_);
          errno = 0;
          update_time();
          this->new_name();
          FILE* tmp = fopen(name_cache_.c_str(), "a+");
          if(!tmp)
          {
            // strerror is not thread safe.
            fprintf(stderr, "open '%s': %s\n", name_cache_.c_str(), strerror(errno));
          }
          else
          {
            fclose(fp_);
            this->link();
            fp_ = tmp;
          }
        }
      }
  };

  class StdLog
  {
    public:
      StdLog(Receiver<std::string>& rcv)
        : rcv_(rcv), fp_(stderr)
      {}

      ~StdLog() = default;

      void write()
      {
        while(rcv_.is_working())
        {
          rcv_.recv(data_);
          roll();
        }
        while(rcv_.try_recv(data_))
          roll();
      }

    private:
      Receiver<std::string>& rcv_;
      FILE* fp_;
      std::string data_;

      void roll()
      {
        const char* data = data_.data();
        size_t rest = data_.size();
        size_t write_bytes = 0;
        while(rest != 0)
        {
          write_bytes = fwrite_unlocked(data, 1, rest, fp_);
          data += write_bytes;
          rest -= write_bytes;
        }
        data_.clear();
      }
  };

  using Ptype = std::unique_ptr<std::thread, std::function<void(std::thread*)>>;
  static auto pair = channel<std::string>();
  static Ptype backgroud;
  static bool ok = true;

  static void create_logger(const char* path, const char* prefix,
                            long interval)
  {
    if(LOG_DISABLED)
      return;
    char* res = getenv("LOG_TYPE");
    std::string env;
    if(res != nullptr)
      env = res;
    auto& rx = std::get<1>(pair);
    if(env != "stdout")
    {
      if(path == nullptr)
        path = "";
      if(prefix == nullptr)
        prefix = "";
      auto log = std::make_shared<FileLog>(rx, path, prefix, interval);
      if(!log->init())
      {
        ok = false;
        return;
      }
      backgroud = Ptype(new std::thread([log] {
        log->write();
      }), [&rx](std::thread* tid) {
        rx.singal();
        try { tid->join(); } catch(const std::system_error&) {}
        delete tid;
      });
    }
    else
    {
      auto log = std::make_shared<StdLog>(rx);
      backgroud = Ptype(new std::thread([log] {
        log->write();
      }), [&rx](std::thread* tid){
        rx.singal();
        try { tid->join(); } catch(const std::system_error&) {}
        delete tid;
      });
    }
    if(ok)
      rx.init(); // make channel valid.
  }

  bool Logger::create(const char* path,
                        const char* prefix, long interval)
  {
    static std::once_flag g_once;
    std::call_once(g_once, create_logger, path, prefix, interval);
    return ok;
  }

  thread_local std::stringstream Logger::ss_{""};

  Logger::Logger(const char* file, long line, const char* func, Level level)
    : stream_(nullptr)
  {
    auto& tx = std::get<0>(pair);
    stream_.reset(new Stream{tx});
    ss_ << " " << get_tid()
            << MSG[level] << basename(file) << ":" << line << " ";
    if(func)
      ss_ << "`" << func << "` ";
  }

  Logger::~Logger()
  {
    if(stream_)
    {
      stream_->finish(ss_);
      ss_.str("");
    }
  }
}