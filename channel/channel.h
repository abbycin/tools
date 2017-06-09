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

class SpinLock;
template<typename> class Guard;
template<typename> class Queue;
template<typename> class Sender;
template<typename> class Receiver;
template<typename T> std::tuple<Sender<T>, Receiver<T>> channel();

// -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
#ifdef TBB_ALLOC // -ltbbmalloc
#include <tbb/scalable_allocator.h>

template<typename T>
T* alloc()
{
  return static_cast<T*>(scalable_malloc(sizeof(T)));
}

void dealloc(void* x)
{
  scalable_free(x);
}
#elif defined(JEMALLOC) // -ljemalloc
#include <jemalloc/jemalloc.h>
template<typename T>
T* alloc()
{
  return static_cast<T*>(malloc(sizeof(T)));
}

void dealloc(void *x)
{
  free(x);
}
#else
template<typename T>
T* alloc()
{
  return new T;
}

template<typename T>
void dealloc(T* x)
{
  delete x;
}
#endif

#ifdef SPIN
using Mutex = SpinLock;
template<typename T> using LockGuard = Guard<T>;
using CondVar = std::condition_variable_any;
#else
using Mutex = std::mutex;
template<typename T> using LockGuard = std::unique_lock<T>;
using CondVar = std::condition_variable;
#endif

#ifdef TBB_QUEUE
#include <tbb/concurrent_queue.h>
template<typename T> using Queue_t = tbb::concurrent_queue<T>;
#else
template<typename T> using Queue_t = Queue<T>;
#endif

class SpinLock
{
  public:
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
      while(!flag_.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
      flag_.clear(std::memory_order_release);
    }

  private:
    std::atomic_flag flag_;
};

template<typename T>
class Guard
{
  public:
    Guard(T& lk)
      : lk_(lk)
    {}

    ~Guard()
    {
      lk_.unlock();
    }

    void lock()
    {
      lk_.lock();
    }

    void unlock()
    {
      lk_.unlock();
    }
  private:
    T& lk_;
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
      head_(alloc<Node>()),
      tail_(head_.load())
    {
      auto tmp = head_.load();
      tmp->next.store(nullptr);
    }

    ~Queue()
    {
      this->clear();
    }

    void clear()
    {
      Node* tmp = nullptr;
      while(tail_)
      {
        tmp = tail_->next.load();
        dealloc(tail_);
        tail_ = tmp;
      }
      tail_ = nullptr;
    }

    bool empty() const
    {
      return size_ == 0;
    }

    size_t unsafe_size() const
    {
      return size_.load();
    }

    void push(const T& data)
    {
      auto tmp = alloc<Node>();
      tmp->data = data;
      tmp->next.store(nullptr, std::memory_order_relaxed);
      auto old_head = head_.exchange(tmp, std::memory_order_acq_rel);
      old_head->next.store(tmp, std::memory_order_release);
      size_ += 1;
    }

    void push(T&& data)
    {
      auto tmp = alloc<Node>();
      tmp->data = std::move(data);
      tmp->next.store(nullptr, std::memory_order_relaxed);
      auto old_head = head_.exchange(tmp, std::memory_order_acq_rel);
      old_head->next.store(tmp, std::memory_order_release);
      size_ += 1;
    }

    bool try_pop(T& data)
    {
      auto next = tail_->next.load(std::memory_order_acquire);
      if(next == nullptr)
        return false;
      data = std::move(next->data);
      dealloc(tail_);
      tail_ = next;
      size_ -= 1;
      return true;
    }

  private:
    std::atomic<size_t> size_;
    std::atomic<Node*> head_;
    Node* tail_;
};

// tbbmalloc + spinlock > jemalloc + spinlock > glibc malloc + spinlock
// spinlock > std::mutex
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

    ~ReceiverImpl() = default;

    void send(const T& data)
    {
      queue_.push(data);
      if(!state_)
      {
        cond_.notify_one();
      }
    }

    void send(T&& data)
    {
      queue_.push(std::move(data));
      if(!state_)
      {
        cond_.notify_one();
      }
    }

    bool try_recv(T& data)
    {
      return queue_.try_pop(data);
    }

    void recv(T& data)
    {
      for(;;)
      {
        if(!state_)
        {
          LockGuard<Mutex> lk(mtx_);
          cond_.wait(lk, [this] {
              return !queue_.empty();
            });
        }
        if(queue_.try_pop(data))
        {
          state_ = true;
          break;
        }
        state_ = false;
      }
    }

    template<typename Rep, typename Period>
    bool recv_timeout(T& data, const std::chrono::duration<Rep, Period>& timeout)
    {
      LockGuard<Mutex> lk(mtx_);
      cond_.wait_for(lk, timeout, [this] {
            return !queue_.empty();
          });
      return queue_.try_pop(data);
    }

  private:
    Queue_t<T> queue_;
    Mutex mtx_;
    CondVar cond_;
    bool state_{false};
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

    ~Sender() = default;

    void send(const T& data)
    {
      sender_->send(data);
    }

    void send(T&& data)
    {
      sender_->send(std::move(data));
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

    Receiver(Receiver&& rhs)
    {
      if(this != &rhs)
      {
        recv_ = rhs.recv_;
        rhs.recv_.reset();
      }
    }

    ~Receiver() = default;

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
      return recv_->recv_timeout(data, timeout);
    }

  private:
    Receiver(ReceiverImpl<T>* recv)
      : recv_(recv)
    {}

    Receiver(const Receiver&) = delete;

    Receiver& operator= (const Receiver&) = delete;

    std::shared_ptr<ReceiverImpl<T>> get()
    {
      return recv_;
    }

    std::shared_ptr<ReceiverImpl<T>> recv_;
};

template<typename T>
std::tuple<Sender<T>, Receiver<T>> channel()
{
  Receiver<T> receiver(new ReceiverImpl<T>());
  Sender<T> sender(receiver.get());
  return std::make_tuple(std::move(sender), std::move(receiver));
}

#endif // CHANNEL_H_
