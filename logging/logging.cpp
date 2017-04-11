/*********************************************************
          File Name: logging.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 08 Apr 2017 11:13:12 PM CST
**********************************************************/

#include "logging.h"
#include "mpsc.h"
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

  inline void build_time(timeval& tv_, tm& tm_, char* time_buf_, size_t size)
  {
    gettimeofday(&tv_, nullptr);
    localtime_r(&tv_.tv_sec, &tm_);
    snprintf(time_buf_, size, "%04d%02d%02d_%02d:%02d:%02d.%06ld",
             tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday,
             tm_.tm_hour, tm_.tm_min, tm_.tm_sec, tv_.tv_usec);
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
      Stream(Sender<std::string>& s, Logger::Level level,
             const char* file, long line, const char* func = nullptr)
        : sender_(s)
      {
        build_time(tv_, tm_, time_buf_, sizeof(time_buf_));
        buffer_ << time_buf_ << " " << get_tid()
                << MSG[level] << basename(file) << ":" << line << " ";
        if(func)
          buffer_ << "`" << func << "` ";
      }

      ~Stream() = default;

      void finish()
      {
        buffer_ << "\n";
        sender_.send(buffer_.str());
      }

      std::stringstream& stream()
      {
        return buffer_;
      }

    private:
      Sender<std::string>& sender_;
      std::stringstream buffer_;
      char time_buf_[25];
      timeval tv_;
      tm tm_;
  };

  class FileLog
  {
    public:
      FileLog(Receiver<std::string>& rx, const std::string& path,
              const std::string& prefix, long interval)
        : rcv_(rx), path_(path), saved_path_(path), interval_(interval),
          count_(COUNT_DOWN), time_cache_(0), fp_(nullptr)
      {
        if(path_.empty())
          path_ = "./";
        auto pos = path_.find_last_of("/");
        if(pos == path_.npos)
          path_ += "/";
        else
          path_.erase(pos + 1);
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
        update_time();
        time_cache_ = now();
        fp_ = fopen(new_name().c_str(), "a+");
        if(fp_ == nullptr)
        {
          fprintf(stderr, "open '%s': %s\n", saved_path_.c_str(), strerror(errno));
          return false;
        }
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
      const std::string saved_path_;
      const long interval_;
      int count_;
      long time_cache_;
      FILE* fp_;
      std::string data_;
      char time_buf_[25];
      char pid_buf_[10];
      timeval tv_;
      tm tm_;

      inline void update_time()
      {
        build_time(tv_, tm_, time_buf_, sizeof(time_buf_));
      }

      // milliseconds
      inline long now()
      {
        return tv_.tv_sec * 1000 + tv_.tv_usec / 1000; 
      }

      inline std::string new_name()
      {
        memset(pid_buf_, 0, sizeof(pid_buf_));
        sprintf(pid_buf_, "%d", getpid());
        std::string res{path_ + time_buf_};
        res.append("_").append(pid_buf_).append(".log");
        return res;
      }

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
        if(count_-- >= 0)
          return;
        fflush_unlocked(fp_);
        count_ = COUNT_DOWN;
        if(interval_ <= 0)
          return;
        update_time();
        auto cur = now();
        if(cur - time_cache_ > interval_)
        {
          time_cache_ = cur;
          fflush(fp_);
          fclose(fp_);
          errno = 0;
          fp_ = fopen(new_name().c_str(), "a+");
          if(!fp_)
            fprintf(stderr, "open '%s': %s\n", saved_path_.c_str(), strerror(errno));
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

  Logger::Logger(Logger&& rhs)
  {
    if(this != &rhs)
    {
      this->stream_ = rhs.stream_;
      rhs.stream_ = nullptr;
    }
  }

  Logger::~Logger()
  {
    if(stream_)
    {
      stream_->finish();
      delete stream_;
    }
  }

  Logger::Wrapper Logger::stream()
  {
    return stream_->stream();
  }

  using Ptype = std::unique_ptr<std::thread, std::function<void(std::thread*)>>;
  static auto pair = channel<std::string>();
  static Ptype backgroud{};
  static std::once_flag g_once;
  static bool ok = true;

  static void create_logger(const std::string& path, const std::string& prefix,
                            long interval)
  {
    auto& rx = std::get<1>(pair);
    char* env = getenv("LOG_TYPE");
    if(env == nullptr || std::string(env) != "STDOUT")
    {
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
  }

  bool Logger::create(const std::string& path,
                        const std::string& prefix, long interval)
  {
    std::call_once(g_once, create_logger, path, prefix, interval);
    return ok;
  }

  Logger::Logger(const char* file, long line, const char* func, Level level)
    : stream_(nullptr)
  {
    if(!backgroud)
      throw std::runtime_error("invalide receiver");
    auto& tx = std::get<0>(pair);
    stream_ = new Stream{tx, level, file, line, func};
  }
}
