/*********************************************************
          File Name: logging.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 08 Apr 2017 11:13:12 PM CST
**********************************************************/

#include "logging.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <sys/time.h>
#include <syscall.h>

namespace nm
{
  namespace meta
  {
    class Sender;
    class Receiver;
    std::pair<Sender, Receiver> channel();

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
          : size_(0),
            head_(new Node()),
            tail_(head_.load())
        {
          auto tmp = head_.load();
          tmp->next.store(nullptr);
        }

        ~Queue()
        {
          auto tmp = head_.load();
          Node* cur = nullptr;
          while(tmp != nullptr)
          {
            cur = tmp;
            tmp = tmp->next.load();
            delete cur;
          }
        }

        bool empty() const
        {
          return size_ == 0;
        }

        void push(const std::string& data)
        {
          auto tmp = new Node();
          tmp->data = data;
          tmp->next.store(nullptr, std::memory_order_relaxed);
          auto old_head = head_.exchange(tmp, std::memory_order_acq_rel);
          old_head->next.store(tmp, std::memory_order_release);
          size_ += 1;
        }

        void push(std::string&& data)
        {
          auto tmp = new Node();
          tmp->data = std::move(data);
          tmp->next.store(nullptr, std::memory_order_relaxed);
          auto old_head = head_.exchange(tmp, std::memory_order_acq_rel);
          old_head->next.store(tmp, std::memory_order_release);
          size_ += 1;
        }

        bool try_pop(std::string& data)
        {
          auto next = tail_->next.load(std::memory_order_acquire);
          if(next == nullptr)
            return false;
          data = std::move(next->data);
          delete tail_;
          tail_ = next;
          size_ -= 1;
          return true;
        }

      private:
        std::atomic<size_t> size_;
        std::atomic<Node*> head_;
        Node* tail_;
    };

    class ReceiverImpl
    {
      private:
        ReceiverImpl()
          : real_queue_(new Queue()), queue_(nullptr)
        {}

      public:
        friend std::pair<Sender, Receiver> channel();

        ReceiverImpl(const ReceiverImpl&) = delete;

        ReceiverImpl& operator= (const ReceiverImpl&) = delete;

        ReceiverImpl(ReceiverImpl&& rhs) = delete;

        ~ReceiverImpl()
        {
          if(real_queue_)
            delete real_queue_;
        }
        
        void send(const std::string& data)
        {
          queue_->push(data);
          if(!state_)
          {
            cond_.notify_one();
          }
        }

        void send(std::string&& data)
        {
          queue_->push(std::move(data));
          if(!state_)
          {
            cond_.notify_one();
          }
        }

        bool try_recv(std::string& data)
        {
          return queue_->try_pop(data);
        }

        void recv(std::string& data)
        {
          for(;switcher_;)
          {
            if(!state_)
            {
              std::unique_lock<std::mutex> lk(mtx_);
              cond_.wait(lk, [this] {
                return !switcher_ || !queue_->empty();
              });
            }
            if(queue_->try_pop(data))
            {
              state_ = true;
              break;
            }
            state_ = false;
          }
        }

        void signal()
        {
          // modify the shared variable must under the mutex in order to correctly
          // publish the modification to the waiting thread, Even if the shared
          // variable is atomic
          std::lock_guard<std::mutex> lg(mtx_);
          switcher_ = false;
          cond_.notify_one();
        }

        bool is_working()
        {
          return switcher_;
        }

        void init()
        {
          std::call_once(once_, [this] {
            queue_.reset(real_queue_);
            real_queue_ = nullptr;
          });
        }

      private:
        Queue* real_queue_;
        std::unique_ptr<Queue> queue_;
        std::mutex mtx_;
        std::condition_variable cond_;
        bool state_{false};
        bool switcher_{true};
        std::once_flag once_;
    };

    class Sender
    {
      public:
        friend std::pair<Sender, Receiver> channel();

        Sender(Sender&& rhs)
        {
          sender_ = rhs.sender_;
          rhs.sender_.reset();
        }

        ~Sender() = default;
        
        void send(const std::string& data)
        {
          sender_->send(data);
        }

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
        friend std::pair<Sender, Receiver> channel();

        Receiver(Receiver&& rhs)
        {
          if(this != &rhs)
          {
            recv_ = rhs.recv_;
            rhs.recv_.reset();
          }
        }

        ~Receiver() = default;

        void recv(std::string& data)
        {
          recv_->recv(data);
        }

        bool try_recv(std::string& data)
        {
          return recv_->try_recv(data);
        }

        void singal()
        {
          recv_->signal();
        }

        bool is_working()
        {
          return recv_->is_working();
        }

        void init()
        {
          recv_->init();
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

    std::pair<Sender, Receiver> channel()
    {
      ReceiverImpl* recv = new ReceiverImpl();
      Receiver receiver(recv);
      Sender sender(receiver.get());
      return {std::move(sender), std::move(receiver)};
    }

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
  }
}

namespace nm
{
  using namespace meta;

  class FileLog
  {
    public:
      FileLog(const std::string& path,
              const std::string& prefix, long interval)
        : path_(path), interval_(interval), count_(now()),
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

      FileLog()
        : path_(), interval_(-1), count_(now()),
          time_cache_(count_), last_roll_(count_), fp_(stderr)
      {}

      virtual ~FileLog()
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
            fprintf(stderr, "open '%s': %s\n", name_cache_.c_str(), strerror(errno));
            return false;
          }
          this->link();
        }
        return true;
      }

      void write(std::string&& data)
      {
        std::lock_guard<std::mutex> lg(mtx_);
        data_ = std::move(data);
        roll();
      }

    private:
      enum { COUNT_DOWN = 3 };
      std::mutex mtx_;
      std::string path_;
      const long interval_;
      long count_;
      long time_cache_;
      long last_roll_;
      FILE* fp_;
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

    protected:
      std::string data_;

      virtual void roll()
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
        if(time_cache_ - count_ >= COUNT_DOWN)
        {
          count_ = time_cache_;
          fflush_unlocked(fp_);
        }
        if(interval_ <= 0)
          return;
        if(time_cache_ - last_roll_ >= interval_)
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

  class FileLogAsync : public FileLog
  {
    public:
      FileLogAsync(Receiver& rx, const std::string& path,
      const std::string& prefix, long interval)
      : FileLog(path, prefix, interval), rcv_(rx)
        {}

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
      Receiver& rcv_;
  };

  class StdLog : public FileLog
  {
    public:
      StdLog(Receiver& rcv)
        : FileLog(), rcv_(rcv)
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
      Receiver& rcv_;
  };

  using Ptype = std::unique_ptr<std::thread, std::function<void(std::thread*)>>;
  static auto mpsc = channel();
  static std::unique_ptr<FileLog> g_log;
  static Ptype backgroud;
  static std::once_flag g_once;
  static bool ok = true;

  static void create_logger(const char* path, const char* prefix,
                            long interval, bool is_sync)
  {
    if(LOG_DISABLED)
      return;
    if(path == nullptr)
      path = "";
    if(prefix == nullptr)
      prefix = "";
    char* res = getenv("LOG_TYPE");
    std::string env;
    if(res != nullptr)
      env = res;
    auto& rx = mpsc.second;
    if(is_sync)
    {
      if(env != "stdout")
      {
        g_log.reset(new FileLog(path, prefix, interval));
        if(!g_log->init())
        {
          ok = false;
          return;
        }
      }
      else
      {
        g_log.reset(new StdLog(rx));
        g_log->init();
      }
      return;
    }
    if(env != "stdout")
    {
      g_log.reset(new FileLogAsync(rx, path, prefix, interval));
      if(!g_log->init())
      {
        ok = false;
        return;
      }
      backgroud = Ptype(new std::thread([] {
        auto log = static_cast<FileLogAsync*>(g_log.get());
        log->write();
      }), [&rx](std::thread* tid) {
        rx.singal();
        try { tid->join(); } catch(const std::system_error&) {}
        delete tid;
      });
    }
    else
    {
      g_log.reset(new StdLog(rx));
      g_log->init();
      backgroud = Ptype(new std::thread([] {
        auto log = static_cast<StdLog*>(g_log.get());
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

  bool Logger::create_async(const char* path, const char* prefix, long interval)
  {
    std::call_once(g_once, create_logger, path, prefix, interval, false);
    if(!cb_)
      cb_ = [](std::stringstream& ss) { mpsc.first.send(ss.str()); };
    return ok;
  }

  bool Logger::create_sync(const char* path, const char* prefix, long interval)
  {
    std::call_once(g_once, create_logger, path, prefix, interval, true);
    if(!cb_)
      cb_ = [](std::stringstream& ss) { g_log->write(ss.str()); };
    return ok;
  }

  thread_local std::stringstream Logger::ss_{""};
  std::function<void(std::stringstream&)> Logger::cb_{};

  Logger::Logger(const char* file, long line, const char* func, Level level)
  {
    ss_ << " " << get_tid()
            << MSG[level] << basename(file) << ":" << line << " ";
    if(func)
      ss_ << "`" << func << "` ";
  }

  Logger::~Logger()
  {
    ss_ << "\n";
    cb_(ss_);
    ss_.str("");
  }
}
