/***********************************************
        File Name: client.cpp
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 9/9/16 7:51 PM
***********************************************/

#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio::ip;
using boost::asio::io_service;
using boost::system::error_code;
using boost::asio::read_until;
using boost::asio::write;

using Callback = std::function<void(const std::string&)>;

class Client : boost::noncopyable
{
  public:
    Client(io_service& io, tcp::endpoint&& ep)
      : socket_(io, std::move(ep.protocol())),
        cb_()
    {
      socket_.connect(std::move(ep));
    }

    ~Client()
    {
      socket_.shutdown(socket_.shutdown_both);
      socket_.close();
    }

    std::string request()
    {
      write(socket_, boost::asio::buffer("ping\n"));
      read_until(socket_, buf_, '\n');
      std::istream in(&buf_);
      std::string msg;
      std::getline(in, msg);
      return msg;
    }

  private:
    tcp::socket socket_;
    Callback cb_;
    boost::asio::streambuf buf_;
};

int main()
{
  io_service io;
  Client cl(io, tcp::endpoint(address::from_string("127.0.0.1"), 8888));

  try
  {
    std::cout << cl.request() << std::endl;
  }
  catch(const boost::system::system_error& e)
  {
    std::cerr << e.what() << std::endl;
  }
}
