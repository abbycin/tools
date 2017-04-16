/*********************************************************
          File Name: mpsc.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 08 Apr 2017 07:03:55 PM CST
**********************************************************/

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <tuple>

// modified channel.h
namespace nm
{
  template<typename> class Sender;
  template<typename> class Receiver;
  template<typename T> std::tuple<Sender<T>, Receiver<T>> channel();

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
        head_(new Node()),
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

      void push(const T& data)
      {
        auto tmp = new Node();
        tmp->data = data;
        tmp->next.store(nullptr, std::memory_order_relaxed);
        auto old_head = head_.exchange(tmp, std::memory_order_acq_rel);
        old_head->next.store(tmp, std::memory_order_release);
        size_ += 1;
      }

      void push(T&& data)
      {
        auto tmp = new Node();
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

  template<typename T>
  class ReceiverImpl
  {
    private:
      ReceiverImpl()
        : real_queue_(new Queue<T>()), queue_(nullptr)
      {}

    public:
      template<typename U> friend std::tuple<Sender<U>, Receiver<U>> channel();

      ReceiverImpl(const ReceiverImpl&) = delete;

      ReceiverImpl& operator= (const ReceiverImpl&) = delete;

      ReceiverImpl(ReceiverImpl&& rhs) = delete;

      ~ReceiverImpl()
      {
        if(real_queue_)
          delete real_queue_;
      }

      void send(const T& data)
      {
        queue_->push(data);
        if(!state_)
        {
          cond_.notify_one();
        }
      }

      void send(T&& data)
      {
        queue_->push(std::move(data));
        if(!state_)
        {
          cond_.notify_one();
        }
      }

      bool try_recv(T& data)
      {
        return queue_->try_pop(data);
      }

      void recv(T& data)
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
      Queue<T>* real_queue_;
      std::unique_ptr<Queue<T>> queue_;
      std::mutex mtx_;
      std::condition_variable cond_;
      bool state_{false};
      bool switcher_{true};
      std::once_flag once_;
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
    ReceiverImpl<T>* recv = new ReceiverImpl<T>;
    Receiver<T> receiver(recv);
    Sender<T> sender(receiver.get());
    return std::make_tuple(std::move(sender), std::move(receiver));
  }
} // namespace nm

#endif // CHANNEL_H_
