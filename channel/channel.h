/*********************************************************
          File Name: channel.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 25 Feb 2017 09:01:47 PM CST
**********************************************************/

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <tuple>

class SpinLock
{
  public:
    template<typename T>
    class Guard
    {
      public:
        Guard(T& lk)
          : lk_(lk)
          {
            lk_.lock();
          }

          ~Guard()
          {
            lk_.unlock();
          }
        private:
          T& lk_;
      };

    SpinLock()
      : flag_{ 0 }
    {}

    ~SpinLock()
    {
      flag_.clear(std::memory_order_release);
    }

    void try_lock()
    {
      flag_.test_and_set(std::memory_order_acquire);
    }

    void lock()
    {
      while (!flag_.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
      flag_.clear(std::memory_order_release);
    }

  private:
    std::atomic_flag flag_;
};

template<typename T>
class Queue
{
  private:
    struct Node
    {
      T data;
      std::atomic<Node*> next;
    };

  public:
    Queue()
      : size_(0),
      head_(new Node),
      tail_(head_.load())
    {
      auto tmp = head_.load();
      tmp->next.store(nullptr);
    }

    ~Queue()
    {
      auto tmp = head_.load();
      auto cur = tmp;
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

    size_t size() const
    {
      return size_.load();
    }

    void push(const T& data)
    {
      auto tmp = new Node;
      tmp->data = data;
      tmp->next.store(nullptr, std::memory_order_relaxed);
      auto old_head = head_.exchange(tmp, std::memory_order_acq_rel);
      old_head->next.store(tmp, std::memory_order_release);
      size_ += 1;
    }

    bool pop(T& data)
    {
      auto tail = tail_.load(std::memory_order_release);
      auto next = tail->next.load(std::memory_order_acquire);
      if(next == nullptr)
        return false;
      data = next->data;
      tail_.store(next, std::memory_order_acquire);
      delete tail;
      size_ -= 1;
      return true;
    }

  private:
    std::atomic<size_t> size_;
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
};

template<typename> class Sender;

template<typename> class Receiver;

template<typename T>
std::tuple<Sender<T>, Receiver<T>> channel();

template<typename T>
class ReceiverImpl
{
  private:
    ReceiverImpl() = default;

  public:
    template<typename U> friend std::tuple<Sender<U>, Receiver<U>> channel();

    ReceiverImpl(const ReceiverImpl&) = delete;

    ReceiverImpl& operator= (const ReceiverImpl&) = delete;

    ReceiverImpl(ReceiverImpl&& rhs) = delete;

    void send(const T& data)
    {
      queue_.push(data);
      if(queue_.size() > 128)
        return;
      std::lock_guard<std::mutex> guard(mtx_);
      cond_.notify_one();
    }

    bool try_recv(T& data)
    {
      return queue_.pop(data);
    }

    void recv(T& data)
    {
      std::unique_lock<std::mutex> lk(mtx_);
      cond_.wait(lk, [this] {
            return !queue_.empty();
          });
      while(!queue_.pop(data));
    }

    template<typename Rep, typename Period>
    bool recv_timeout(T& data, const std::chrono::duration<Rep, Period>& timeout)
    {
      std::unique_lock<std::mutex> lk(mtx_);
      cond_.wait_for(lk, timeout, [this] {
            return !queue_.empty();
          });
      return queue_.pop(data);
    }

  private:
    Queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cond_;
    //std::condition_variable_any cond_;
};

template<typename T>
class Sender
{
  public:
    template<typename U> friend std::tuple<Sender<U>, Receiver<U>> channel();

    Sender(Sender&& rhs)
    {
      sender_ = rhs.sender_;
      rhs.sender_.reset();
    }

    ~Sender()
    {}

    void send(const T& data)
    {
      sender_->send(data);
    }

    Sender clone()
    {
      return *this;
    }

  private:
    Sender(std::shared_ptr<ReceiverImpl<T>> recv)
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

    std::shared_ptr<ReceiverImpl<T>> sender_;
};

template<typename T>
class Receiver
{
  public:
    template<typename U> friend std::tuple<Sender<U>, Receiver<U>> channel();
    friend class Sender<T>;

    Receiver(Receiver&& rhs)
    {
      if(this != &rhs)
      {
        recv_ = rhs.recv_;
        rhs.recv_.reset();
      }
    }

    void recv(T& data)
    {
      recv_->recv(data);
    }

    bool try_recv(T& data)
    {
      return recv_->try_recv(data);
    }

    template<typename Rep, typename Period>
    bool recv_timeout(T& data, const std::chrono::duration<Rep, Period>& timeout)
    {
      return recv_.recv_timeout(data, timeout);
    }

  private:
    Receiver(ReceiverImpl<T>* recv)
      : recv_(recv)
    {}

    Receiver(const Receiver&) = delete;

    Receiver& operator= (const Receiver&) = delete;

    void send(const T& data)
    {
      recv_->send(data);
    }

    std::shared_ptr<ReceiverImpl<T>> get()
    {
      return recv_;
    }

    std::shared_ptr<ReceiverImpl<T>> recv_;
};

template<typename T>
std::tuple<Sender<T>, Receiver<T>> channel()
{
  ReceiverImpl<T>* recv = new ReceiverImpl<T>;
  Receiver<T> receiver(recv);
  Sender<T> sender(receiver.get());
  return std::make_tuple(std::move(sender), std::move(receiver));
}

#endif // CHANNEL_H_
