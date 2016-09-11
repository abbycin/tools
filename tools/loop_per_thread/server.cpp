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
#include <queue>

using boost::asio::io_service;
using namespace boost::asio::ip;
using boost::system::error_code;
using boost::asio::async_read_until;
using boost::asio::async_write;

using Callback = std::function<void()>;

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
    {}

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
      async_read_until(socket_, buf_, '\n',
                       std::bind(&Session::processing,
                                 shared_from_this(),
                                 std::placeholders::_1,
                                 std::placeholders::_2));
    }

    void processing(const error_code& e, size_t)
    {
      if(e)
      {
        std::cerr << e.message() << std::endl;
        return;
      }
      //std::this_thread::sleep_for(std::chrono::seconds(3)); // simulate processing
      auto self(shared_from_this());
      async_write(socket_, boost::asio::buffer("pong\n"),
                  [this, self](const error_code& ec, size_t) // pervert syntax
                  {
                    if(ec)
                      std::cerr << ec.message() << std::endl;
                    else
                      do_read();
                    /*
                    if(cb_)
                      cb_();
                    socket_.shutdown(boost::asio::socket_base::shutdown_both);
                     */
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
    : rbuf_(0),
      wbuf_(0),
      reader_(io_),
      writer_(io_),
      read_buf_(&rbuf_, sizeof rbuf_),
      write_buf_(&wbuf_, sizeof wbuf_),
      thread_(std::bind(&Worker::thread_func, this))
    {

    }

    ~Worker()
    {
      if(thread_.joinable())
        thread_.join();
    }

    io_service& get_io_service()
    {
      return io_;
    }

    void enqueue(SessionPtr session)
    {
      enqueue_writer(session);
    }

    void stop()
    {
      std::lock_guard<std::mutex> lg(mtx_);
      io_.stop();
    }

  private:
    char rbuf_, wbuf_;
    io_service io_;
    boost::asio::local::stream_protocol::socket reader_, writer_;
    boost::asio::mutable_buffers_1 read_buf_;
    boost::asio::const_buffers_1 write_buf_;
    std::thread thread_;
    std::queue<SessionPtr> tasks_;
    std::mutex mtx_;

    void enqueue_reader()
    {
      reader_.async_read_some(read_buf_,
                              [this](const error_code& ec, size_t bytes)
                              {
                                read_handle(ec, bytes);
                              });
    }

    void enqueue_writer(SessionPtr& session)
    {
      std::lock_guard<std::mutex> lg(mtx_);
      tasks_.push(session);
      writer_.write_some(write_buf_);
    }

    void read_handle(const error_code& ec, size_t)
    {
      if(ec)
        return;
      std::lock_guard<std::mutex> lg(mtx_);
      while(!tasks_.empty())
      {
        auto session = tasks_.front();
        session->run();
        tasks_.pop();
      }
      enqueue_reader();
    }

    void thread_func()
    {
      boost::asio::local::connect_pair(reader_, writer_);
      enqueue_reader();
      io_.run();
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
    {
    }

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

    void accept_handle(SessionPtr& session,
                       WorkerPtr& worker,
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
                             std::bind(&Manager::accept_handle,
                                       this,
                                       session,
                                       worker,
                                       std::placeholders::_1));
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

int main()
{
  try
  {
    Server se(tcp::endpoint(address::from_string("127.0.0.1"), 8888), 4);
    se.run();
  }
  catch(const boost::system::system_error& e)
  {
    std::cerr << e.what() << std::endl;
  }
}
