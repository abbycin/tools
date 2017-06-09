/*********************************************************
          File Name: logging.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 08 Apr 2017 07:05:39 PM CST
**********************************************************/

#ifndef LOGGING_H_
#define LOGGING_H_

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <stdlib.h>
#include <sys/time.h>
#ifndef __linux__
#include <sys/syscall.h>
#else
#include <syscall.h>
#endif
#ifdef __FreeBSD__
#include <sys/thr.h>
#endif
#include <unistd.h>

#ifndef __linux__

#define fwrite_unlocked fwrite
#define fflush_unlocked fflush

const char* basename(const char* name)
{
  if(name == nullptr)
    return nullptr;
  const char* p = name;
  while(*name)
  {
    if(*name++ == '/')
      p = name;
  }
  return p;
}
#endif

namespace nm
{
  namespace meta
  {
    class Queue
    {
      private:
        struct Node
        {
          std::string data;
          std::atomic<Node*> next;
        };

      public:
        Queue()
          : head_(new Node()),
            tail_(head_.load())
        {
          auto tmp = head_.load();
          tmp->next.store(nullptr);
        }

        ~Queue()
        {
          Node* tmp = nullptr;
          while(tail_)
          {
            tmp = tail_->next.load();
            delete tail_;
            tail_ = tmp;
          }
          tail_ = nullptr;
        }

        void push(std::string&& data)
        {
          auto tmp = new Node();
          tmp->data = std::move(data);
          tmp->next.store(nullptr, std::memory_order_relaxed);
          auto old_head = head_.exchange(tmp, std::memory_order_acq_rel);
          old_head->next.store(tmp, std::memory_order_release);
        }

        bool try_pop(std::string& data)
        {
          auto next = tail_->next.load(std::memory_order_acquire);
          if(next == nullptr)
            return false;
          data = std::move(next->data);
          delete tail_;
          tail_ = next;
          return true;
        }

      private:
        std::atomic<Node*> head_;
        Node* tail_;
    };

    class Sender;
    class Receiver;
    class LoggerBackend;

    class ReceiverImpl
    {
      private:
        ReceiverImpl() = default;

      public:
        friend LoggerBackend;

        ReceiverImpl(const ReceiverImpl&) = delete;

        ReceiverImpl& operator= (const ReceiverImpl&) = delete;

        ReceiverImpl(ReceiverImpl&& rhs) = delete;

        ~ReceiverImpl() = default;

        void send(std::string&& data)
        {
          queue_.push(std::move(data));
          if(!state_)
          {
            cond_.notify_one();
          }
        }

        bool try_recv(std::string& data)
        {
          return queue_.try_pop(data);
        }

        bool recv(std::string& data)
        {
          if(!switcher_)
            return false;
          if(!queue_.try_pop(data))
          {
            state_ = false;
            std::unique_lock<std::mutex> lk(mtx_);
            cond_.wait(lk, [this, &data] {
              return !switcher_ || queue_.try_pop(data);
            });
          }
          else
          {
            state_ = true;
          }
          return true;
        }

        void signal()
        {
          // modify the shared variable must under the mutex in order to correctly
          // publish the modification to the waiting thread, Even if the shared
          // variable is atomic.
          std::lock_guard<std::mutex> lg(mtx_);
          switcher_ = false;
          cond_.notify_one();
        }

      private:
        Queue queue_;
        std::mutex mtx_;
        std::condition_variable cond_;
        bool state_{false};
        bool switcher_{true};
    };

    class Sender
    {
      public:
        friend LoggerBackend;

        Sender(Sender&& rhs)
        {
          sender_ = rhs.sender_;
          rhs.sender_.reset();
        }

        ~Sender() = default;

        void send(std::string&& data)
        {
          sender_->send(std::move(data));
        }

      private:
        Sender(std::shared_ptr<ReceiverImpl> recv)
          : sender_(recv)
        {}

        Sender(const Sender& rhs)
        {
          if(this != &rhs)
          {
            sender_ = rhs.sender_;
          }
        }

        Sender& operator= (const Sender&) = delete;

        std::shared_ptr<ReceiverImpl> sender_;
    };

    class Receiver
    {
      public:
        friend LoggerBackend;

        Receiver(Receiver&& rhs)
        {
          if(this != &rhs)
          {
            recv_ = rhs.recv_;
            rhs.recv_.reset();
          }
        }

        ~Receiver() = default;

        bool recv(std::string& data)
        {
          return recv_->recv(data);
        }

        bool try_recv(std::string& data)
        {
          return recv_->try_recv(data);
        }

        void signal()
        {
          recv_->signal();
        }

      private:
        Receiver(ReceiverImpl* recv)
          : recv_(recv)
        {}

        Receiver(const Receiver&) = delete;

        Receiver& operator= (const Receiver&) = delete;

        std::shared_ptr<ReceiverImpl> get()
        {
          return recv_;
        }

        std::shared_ptr<ReceiverImpl> recv_;
    };

    class FileLog
    {
      public:
        FileLog(Receiver& recv, const std::string& path,
                const std::string& prefix, long interval)
          : rcv_(recv), path_(path), interval_(interval), count_(now()),
            time_cache_(count_), last_roll_(count_), fp_(nullptr)
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

        FileLog(Receiver& recv, FILE* fp)
          : rcv_(recv), path_(), interval_(-1), count_(now()),
            time_cache_(count_), last_roll_(count_), fp_(fp)
        {}

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
          update_time();
          if(!fp_)
          {
            errno = 0;
            fp_ = fopen(new_name().c_str(), "a+");
            if(fp_ == nullptr)
            {
              fprintf(stderr, "open '%s': %s\n",
                      name_cache_.c_str(), strerror(errno));
              return false;
            }
            this->link();
            setbuffer(fp_, buffer_, sizeof(buffer_));
          }
          return true;
        }

        void write(std::string&& data)
        {
          std::lock_guard<std::mutex> lg(mtx_);
          data_ = std::move(data);
          roll();
        }

        void write()
        {
          while(rcv_.recv(data_))
          {
            roll();
          }
          while(rcv_.try_recv(data_))
          {
            roll();
          }
        }

      private:
        enum { COUNT_DOWN = 3 };
        Receiver& rcv_;
        std::mutex mtx_;
        std::string path_;
        const long interval_;
        long count_;
        long time_cache_;
        long last_roll_;
        FILE* fp_;
        std::string name_cache_;
        std::string current_log_;
        std::string data_{};
        char buffer_[64 * 1024];
        char time_buf_[25];
        char pid_buf_[10];
        timeval tv_;
        tm tm_;

        void link()
        {
          unlink(current_log_.c_str());
          int res = symlink(new_name().c_str(), current_log_.c_str());
          (void)res;
        }

        void update_time()
        {
          localtime_r(&tv_.tv_sec, &tm_);
          snprintf(time_buf_, sizeof(time_buf_),
                   "%04d%02d%02d_%02d:%02d:%02d.%06ld",
                   tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday,
                   tm_.tm_hour, tm_.tm_min, tm_.tm_sec, tv_.tv_usec);

        }

        void update_micro()
        {
          snprintf(time_buf_ + 18, 7, "%06ld", tv_.tv_usec);
        }

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
          if(time_cache_ - count_ > COUNT_DOWN)
          {
            count_ = time_cache_;
            fflush_unlocked(fp_);
          }
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
              fprintf(stderr, "open '%s': %s\n",
                      name_cache_.c_str(), strerror(errno));
            }
            else
            {
              fclose(fp_);
              this->link();
              fp_ = tmp;
              setbuffer(fp_, buffer_, sizeof(buffer_));
            }
          }
        }
    };

    template<size_t SIZE = 4096>
    class Stream
    {
        template<size_t N> class Buffer
        {
          public:
            Buffer() : pos_(0), count_(0) {}
            ~Buffer() {}

            void append(const char* s, size_t n)
            {
              if(n + pos_ < N)
              {
                std::copy(s, s + n, buffer_ + pos_);
                pos_ += n;
              }
              count_ += n;
            }

            void clear()
            {
              pos_ = 0;
              count_ = 0;
            }

            // drop message silently when it's too long.
            std::string to_string()
            {
              if(count_ >= N)
              {
                return {};
              }
              return {buffer_, pos_};
            }
          private:
            size_t pos_;
            size_t count_;
            char buffer_[N];
        };
      public:
        Stream() = default;
        ~Stream() = default;
        Stream(const Stream&) = delete;
        Stream(Stream&&) = delete;
        Stream& operator= (const Stream&) = delete;
        Stream& operator= (Stream&&) = delete;

        Stream& operator<< (const std::nullptr_t&)
        {
          return *this;
        }

        Stream& operator<< (const char* s)
        {
          if(s)
          {
            buffer_.append(s, std::char_traits<char>::length(s));
          }
          return *this;
        }

        Stream& operator<< (const unsigned char* s)
        {
          return this->operator<<(reinterpret_cast<const char*>(s));
        }

        Stream& operator<< (const std::string& data)
        {
          buffer_.append(data.c_str(), data.size());
          return *this;
        }

        Stream& operator<< (const char c)
        {
          buffer_.append(&c, 1);
          return *this;
        }

        Stream& operator<< (const short data)
        {
          return this->fmt(data);
        }

        Stream& operator<< (const int data)
        {
          return this->fmt(data);
        }

        Stream& operator<< (const long data)
        {
          return this->fmt(data);
        }

        Stream& operator<< (const long long data)
        {
          return this->fmt(data);
        }

        Stream& operator<< (const unsigned short data)
        {
          return this->fmt(data);
        }

        Stream& operator<< (const unsigned int data)
        {
          return this->fmt(data);
        }

        Stream& operator<< (const unsigned long data)
        {
          return this->fmt(data);
        }

        Stream& operator<< (const unsigned char data)
        {
          if(data > std::numeric_limits<char>::max())
            return this->operator<<(static_cast<unsigned>(data));
          return this->operator<<(static_cast<char>(data));
        }

        Stream& operator<< (const unsigned long long data)
        {
          return this->fmt(data);
        }

        Stream& operator<< (const float data)
        {
          return this->append("%.6g", data);
        }

        Stream& operator<< (const double data)
        {
          return this->append("%.10g", data);
        }

        Stream& operator<< (const void* data)
        {
          auto p = reinterpret_cast<uintptr_t>(data);
          fmt_buf_[0] = '0';
          fmt_buf_[1] = 'x';
          fmt_len_ = convertHex(fmt_buf_ + 2, p);
          buffer_.append(fmt_buf_, fmt_len_ + 2);
          return *this;
        }

        std::string str()
        {
          return buffer_.to_string();
        }

        void clear()
        {
          buffer_.clear();
        }

        void append(const char* s, size_t len)
        {
          buffer_.append(s, len);
        }

      private:
        Buffer<SIZE> buffer_;
        char fmt_buf_[32] = {0};
        int fmt_len_{0};

        // the following convert functions are copied from muduo
        const char* const digits = "9876543210123456789";
        const char* const digitsHex = "0123456789abcdef";
        const char* zero = digits + 9;

        template<typename T>
        size_t convert(char buf[], const T value)
        {
          T i = value;
          char* p = buf;

          do {
            int lsd = static_cast<int>(i % 10);
            i /= 10;
            *p++ = zero[lsd]; // maybe negative index
          } while (i != 0);

          if (value < 0) {
            *p++ = '-';
          }
          *p = '\0';
          std::reverse(buf, p);
          return p - buf;
        }

        size_t convertHex(char buf[], uintptr_t value)
        {
          uintptr_t i = value;
          char* p = buf;

          do
          {
            int lsd = static_cast<int>(i % 16);
            i /= 16;
            *p++ = digitsHex[lsd];
          } while (i != 0);

          *p = '\0';
          std::reverse(buf, p);
          return p - buf;
        }

        template<typename T>
        Stream& fmt(T data)
        {
          static_assert(std::is_integral<T>::value, "integer is required");
          size_t res = this->convert(fmt_buf_, data);
          buffer_.append(fmt_buf_, res);
          return *this;
        }

        Stream& append(const char* fmt, double data)
        {
          fmt_len_ = snprintf(fmt_buf_, sizeof(fmt_buf_), fmt, data);
          buffer_.append(fmt_buf_, fmt_len_);
          return *this;
        }
    };

    using StreamBuffer = Stream<4096>;

    class LoggerBackend
    {
      public:
        LoggerBackend()
          : rcv_(new ReceiverImpl()), snd_(rcv_.get()), log_(nullptr), tid_()
        {}

        ~LoggerBackend()
        {
          this->join();
        }
        LoggerBackend(const LoggerBackend&) = delete;
        LoggerBackend(LoggerBackend&&) = delete;
        LoggerBackend& operator= (const LoggerBackend&) = delete;
        LoggerBackend& operator= (LoggerBackend&&) = delete;

        void init(FILE* fp, const char* path, const char* prefix,
                  long interval, bool is_sync)
        {
          std::call_once(once_, [&, this]
          {
            create_logger(fp, path, prefix, interval, is_sync);
            if(is_sync)
              writer_ = [this](StreamBuffer& ss) { sync_write(ss); };
            else
              writer_ = [this](StreamBuffer& ss) { async_write(ss); };
          });
        }

        bool ok()
        {
          return ok_;
        }

        void consume(StreamBuffer& ss)
        {
          writer_(ss);
        }

        void join()
        {
          if(tid_.joinable())
          {
            rcv_.signal();
            try { tid_.join(); } catch(const std::system_error&) {}
          }
        }

      private:
        Receiver rcv_;
        Sender snd_;
        std::unique_ptr <FileLog> log_;
        std::thread tid_;
        bool ok_{true};
        std::function<void(StreamBuffer & )> writer_;
        std::once_flag once_;

        void sync_write(StreamBuffer& ss)
        {
          log_->write(ss.str());
        }

        void async_write(StreamBuffer& ss)
        {
          snd_.send(ss.str());
        }

        void create_logger(FILE* fp, const char* path, const char* prefix,
                           long interval, bool is_sync)
        {
          if(path == nullptr)
            path = "";
          if(prefix == nullptr)
            prefix = "";
          char* res = getenv("LOG_TYPE");
          std::string env;
          if(res != nullptr)
            env = res;
          bool is_stdout = (env == "custom") && fp != nullptr;
          if(is_sync)
            log_sync(fp, path, prefix, interval, is_stdout);
          else
            log_async(fp, path, prefix, interval, is_stdout);
        }

        void log_sync(FILE* fp, const char* path, const char* prefix,
                      long interval, bool is_stdout)
        {
          if(is_stdout)
          {
            log_.reset(new FileLog(rcv_, fp));
            log_->init();
          }
          else
          {
            log_.reset(new FileLog(rcv_, path, prefix, interval));
            if(!log_->init())
            {
              ok_ = false;
              return;
            }
          }
        }

        void log_async(FILE* fp, const char* path, const char* prefix,
                       long interval, bool is_stdout)
        {
          if(is_stdout)
          {
            log_.reset(new FileLog(rcv_, fp));
            log_->init();
            tid_ = std::thread([this] {
              log_->write();
            });
          }
          else
          {
            log_.reset(new FileLog(rcv_, path, prefix, interval));
            if(!log_->init())
            {
              ok_ = false;
              return;
            }
            tid_ = std::thread([this] {
              log_->write();
            });
          }
        }
    };
  }
}

namespace nm
{
#undef disable_log_
#ifndef NOLOG
#define disable_log_ false
#else
#define disable_log_ true
#endif
  class Logger
  {
    public:
      struct Dummy
      {
        template<typename T> Dummy& operator<< (const T&) { return *this; }
      };
      enum Level { INFO = 0, WARNING, DEBUG, ERR, FATAL };
      // default interval is 24hr in seconds, minimum duration is 1s.
      static bool create_async(FILE* fp = stderr, const char* path = "",
                               const char* prefix = "",
                               long interval = 60 * 60 * 24)
      {
        if(disable_log_)
          return true;
        backend_.init(fp, path, prefix, interval, false);
        return backend_.ok();
      }
      static bool create_sync(FILE* fp = stderr, const char* path = "",
                              const char* prefix = "",
                              long interval = 60 * 60 * 24)
      {
        if(disable_log_)
          return true;
        backend_.init(fp, path, prefix, interval, true);
        return backend_.ok();
      }

      // manually wait async operations complete.
      static void join() { backend_.join(); }

      Logger(const char* file, long line, const char* func, Level level)
      {
        ss_ << ' ' << get_tid()
            << MSG[level] << basename(file) << ':' << line << ' ';
        if(func)
          ss_ << '`' << func << "` ";
      }

      // string literal goes here, and string sequence too,
      // but string sequence without line terminator(i.e. '\0')
      // may cause unexpected result.
      template<size_t LEN> Logger& operator<< (const char (&a)[LEN])
      {
        ss_.append(a, LEN - 1);
        return *this;
      }

      // non string literal and others.
      template<typename T> Logger& operator<< (const T& data)
      {
        ss_ << data;
        return *this;
      }

      ~Logger()
      {
        ss_ << '\n';
        backend_.consume(ss_);
        ss_.clear();
      }

      Logger(const Logger&) = delete;
      Logger(Logger&&) = delete;
      Logger& operator= (const Logger&) = delete;
      Logger& operator= (Logger&&) = delete;

    private:
      thread_local inline static meta::StreamBuffer ss_{};
      static inline meta::LoggerBackend backend_{};
      static inline const char* MSG[] = {
        " [INFO]  ",
        " [WARN]  ",
        " [DEBUG] ",
        " [ERROR] ",
        " [FATAL] "
      };

      int get_tid()
      {
#ifdef __APPLE__
        return static_cast<int>(syscall(SYS_thread_selfid));
#elif defined(__FreeBSD__)
        long res = 0;
        thr_self(&res);
        return static_cast<int>(res);
#else
        return static_cast<int>(syscall(SYS_gettid));
#endif
      }
  };

#ifndef NOLOG
  #define log_info() nm::Logger(__FILE__, __LINE__, nullptr, nm::Logger::INFO)
  #define log_warn() nm::Logger(__FILE__, __LINE__, nullptr, nm::Logger::WARNING)
  #define log_debug() nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::DEBUG)
  #define log_err() nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::ERR)
  #define log_fatal() nm::Logger(__FILE__, __LINE__, __func__, nm::Logger::FATAL)
#else
  #define log_info() nm::Logger::Dummy()
  #define log_warn() log_info()
  #define log_debug() log_info()
  #define log_err() log_info()
  #define log_fatal() log_info()
#endif
}

#endif // LOGGING_H_
