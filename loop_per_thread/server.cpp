/***********************************************
        File Name: server.cpp
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 9/9/16 7:51 PM
***********************************************/

#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <mutex>
#include <sys/wait.h>

using boost::asio::io_service;
using namespace boost::asio::ip;
using boost::system::error_code;
using boost::asio::async_read_until;
using boost::asio::async_write;

using Callback = std::function<void()>;

template<typename T>
class Circle
{
  private:
    struct Node
    {
      T data;
      Node* next;
      Node()
        : data(), next(nullptr)
      {}
    };
  public:
    class iterator
    {
      public:
        iterator(Node* n)
          : data_(n)
        {}

        iterator& operator++ ()
        {
          data_ = data_->next;
          return *this;
        }

        T* operator-> ()
        {
          return &data_->data;
        }

        iterator& operator= (const iterator& rhs)
        {
          if(this == &rhs)
            return *this;
          this->data_ = rhs.data_;
          return *this;
        }

        bool operator== (const iterator& rhs)
        {
          return data_ == rhs.data_;
        }

        bool operator!= (const iterator& rhs)
        {
          return data_ != rhs.data_;
        }

      private:
        Node* data_;
    };
    Circle(size_t capacity)
      : size_(capacity), circle_(nullptr)
    {
      for(size_t i = 0; i < size_; ++i)
      {
        if(circle_ == nullptr)
        {
          circle_ = new Node;
          circle_->next = circle_;
          continue;
        }
        Node* tmp = new Node;
        tmp->next = circle_->next;
        circle_->next = tmp;
      }
    }

    ~Circle()
    {
      auto head = circle_->next;
      Node* tmp = nullptr;
      while(head != circle_)
      {
        tmp = head;
        head = head->next;
        delete tmp;
      }
      delete head;
    }

    iterator iter()
    {
      return iterator(circle_);
    }

    size_t size() const
    {
      return size_;
    }

  private:
    size_t size_;
    Node* circle_;
};


/// \brief User defined business processing unit
/**
 * This class is just a sample class which aim to demostrating
 * how to use the infrastructure 'loop-per-thread', you can
 * define your own 'Session' that has the same structure but with
 * different task processing functions to match your own needs.
 */
class Session : public std::enable_shared_from_this<Session>
{
  public:
    static std::shared_ptr<Session> make_session(io_service& io)
    {
      return std::make_shared<Session>(io);
    }

    Session(io_service& io)
      : socket_(io), cb_()
    {
    }

    ~Session()
    {
      if(socket_.is_open())
        socket_.close();
    }

    tcp::socket& socket()
    {
      return socket_;
    }

    void set_callback(Callback&& cb)
    {
      cb_ = std::move(cb);
    }

    void run()
    {
      do_read();
    }

  private:
    tcp::socket socket_;
    Callback cb_;
    boost::asio::streambuf buf_;

    void do_read()
    {
      // NOTE: client may never send a '\n'
      async_read_until(socket_, buf_, '\n',
                       [this, self = shared_from_this()](const error_code& ec, size_t)
                       {
                         if(ec)
                         {
                           //std::cerr << ec.message() << std::endl;
                           return;
                         }
                         processing();
                       });
    }

    void processing()
    {
      async_write(socket_, buf_,
                  [this, self = shared_from_this()](const error_code& ec, size_t)
                  {
                    if(ec)
                    {
                      //std::cerr << ec.message() << std::endl;
                      return;
                    }
                    do_read();
                    if(cb_)
                      cb_();
                  });
    }
};

using SessionPtr = std::shared_ptr<Session>;

/// \brief This is a slave class which manage a bunch of tasks.
/**
 * This class is designed to enqueue new tasks and dequeue tasks
 * which are about to executing. Also, it has ability to cancel
 * queued tasks from the eventloop.
 */
class Worker
{
  public:
    Worker()
    : circle_(std::thread::hardware_concurrency()),
      iter_(circle_.iter())
    {
      for(size_t i = 0; i < circle_.size(); ++i)
      {
        workers_.emplace_back([&, idx = i]() mutable {
            auto task = circle_.iter();
            while(idx--)
              ++task;
            task->run();
          });
      }
    }

    ~Worker()
    {
      for(auto& t: workers_)
      {
        try
        {
          t.join();
        }
        catch(std::system_error&)
        {
          // swallow exception
        }
      }
    }

    io_service& get_io_service()
    {
      return iter_->get_io_service();
    }

    void enqueue(SessionPtr session)
    {
      dispatch(session);
    }

    void stop()
    {
      for(size_t i = 0; i < circle_.size(); ++i)
      {
        iter_->stop();
        ++iter_;
      }
    }

  private:
    class Task
    {
      public:
        Task()
          : strand_(io_),
            work_(new io_service::work(io_))
        {}

        ~Task()
        {}

        io_service& get_io_service()
        {
          return io_;
        }

        // fuck proactor!
        void dispatch(SessionPtr session)
        {
          strand_.dispatch([session] {
            session->run();
          });
        }

        void stop()
        {
          work_.reset();
          io_.stop();
        }

        void run()
        {
          io_.run();
        }

      private:
        io_service io_;
#if BOOST_VERSION >= 106600
        boost::asio::io_service::strand strand_;
#else
        boost::asio::strand strand_;
#endif
        std::unique_ptr<io_service::work> work_;
    };

    Circle<Task> circle_;
    Circle<Task>::iterator iter_;
    std::vector<std::thread> workers_;

    void dispatch(SessionPtr session)
    {
      iter_->dispatch(session);
      ++iter_;
    }
};

using WorkerPtr = std::shared_ptr<Worker>;

/// \brief This class manage loops on threads
/**
 * - passively wait client's connection
 * - create session for each connection
 * - assign session to worker thread
 */
class Manager
{
  public:
    Manager(io_service& io, tcp::endpoint&& ep, unsigned long num)
      : num_(num),
        cur_(0),
        acceptor_(io, std::move(ep), true)
    {
      for(unsigned long i = 0; i < num_; ++i)
        workers_.emplace_back(std::make_shared<Worker>());

      acceptor_.listen();
      start_accept();
    }

    ~Manager()
    {}

    void stop()
    {
      acceptor_.cancel();
      for(auto& x: workers_)
        x->stop();
    }

  private:
    unsigned long num_;
    unsigned long cur_;
    tcp::acceptor acceptor_;
    std::vector<WorkerPtr> workers_;

    void accept_handle(SessionPtr session,
                       WorkerPtr worker,
                       const error_code& ec)
    {
      if(!ec)
      {
        ++cur_;
        worker->enqueue(session);
        start_accept();
      }
    }

    void start_accept()
    {
      if(cur_ == num_)
        cur_ = 0;
      auto worker = workers_.at(cur_);
      auto session(Session::make_session(worker->get_io_service()));
      acceptor_.async_accept(session->socket(),
                             [this, session, worker](const error_code& ec)
                             {
                               accept_handle(session, worker, ec);
                             });
    }
};

/// \brief A wrapper class for users
/**
 * - supplying eventloop for manager class
 * - monitor specific signals
 * - control manager class's start and stop
 */
class Server
{
  public:
    Server(tcp::endpoint&& ep, unsigned long num = 1)
      : sg_(io_),
        manager_(io_, std::move(ep), num)
    {
      sg_.add(SIGINT);
      sg_.add(SIGQUIT);
      sg_.add(SIGTERM);
      sg_.async_wait([this](const error_code& ec, int sig) {
        if(ec)
          std::cerr << ec.message() << std::endl;
        std::cerr << "Caught signal " << sig << ", exit.\n";
        stop();
      });
    }

    void run()
    {
      io_.run();
    }

    void stop()
    {
      manager_.stop();
      io_.stop();
    }

  private:
    io_service io_;
    boost::asio::signal_set sg_;
    Manager manager_;
};

void loop(int port, int num)
{
  try
  {
    Server se(tcp::endpoint(address_v4::any(), port), num);
    se.run();
  }
  catch(const boost::system::system_error& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

int main(int argc, char* argv[])
{
  if(argc != 4)
  {
    fprintf(stderr, "%s child_num thread_num_per_child start_port\n", argv[0]);
    fprintf(stderr, "Example:\n%s 10 2 8888\ncreate 10 child process, 2 thread"
      " per child, listen port range from 8888 to (8888 + 10)\n", argv[0]);
    return 1;
  }
  int child_num = std::stoi(argv[1]);
  int thread_num = std::stoi(argv[2]);
  int port = std::stoi(argv[3]);
  for(int i = 0; i < child_num; ++i, ++port)
  {
    switch(fork())
    {
      case -1:
        perror("fork");
        return 1;
      case 0:
        loop(port, thread_num);
        return 0;
      default:
        break;
    }
  }
  while(wait(NULL) != -1)
    continue;
}
