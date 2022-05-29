/***********************************************
	File Name: client.cpp
	Author: Abby Cin
	Mail: abbytsing@gmail.com
	Created Time: 9/9/16 7:51 PM
***********************************************/

#include <iostream>
#include <boost/asio.hpp>
#include <threadpool>

using namespace boost::asio::ip;
using boost::asio::io_service;
using boost::system::error_code;
using boost::asio::async_read_until;

using Callback = std::function<void(boost::asio::streambuf &)>;

class Client : boost::noncopyable {
public:
	Client(io_service &io, tcp::endpoint &&ep)
		: repeat_(1000)
		, limit_(1000)
		, old_limit_(limit_)
		, ep_(std::move(ep))
		, socket_(io, ep_.protocol())
		, write_buf_("ping\n", 5)
		, cb_()
	{
		socket_.connect(ep_);
	}

	~Client()
	{
		socket_.shutdown(socket_.shutdown_both);
		socket_.close();
	}

	void set_callback(Callback &&cb)
	{
		cb_ = std::move(cb);
	}

	void run()
	{
		handle_write();
	}

private:
	int repeat_;
	int limit_;
	int old_limit_;
	tcp::endpoint ep_;
	tcp::socket socket_;
	boost::asio::streambuf buf_;
	boost::asio::const_buffers_1 write_buf_;
	Callback cb_;
	std::string msg_;

	void reconnect()
	{
		repeat_ -= 1;
		limit_ = old_limit_;
		socket_.shutdown(socket_.shutdown_both);
		socket_.close();
		socket_.connect(ep_);
		if (repeat_ > 0) {
			handle_write();
		}
	}

	void handle_read()
	{
		async_read_until(socket_,
				 buf_,
				 '\n',
				 [this](const error_code &ec, size_t)
				 {
					 if (!ec) {
						 if (limit_ == 0 &&
						     repeat_ > 0) {
							 reconnect();
						 } else {
							 if (cb_)
								 cb_(buf_);
							 limit_ -= 1;
							 handle_write();
						 }
					 }
				 });
	}

	void handle_write()
	{
		socket_.async_write_some(write_buf_,
					 [this](const error_code &ec, size_t)
					 {
						 if (!ec)
							 handle_read();
					 });
	}
};

int main()
{
	std::threadpool pool { std::launch::deferred };
	auto func = []
	{
		io_service io;
		Client cl(
			io,
			tcp::endpoint(address::from_string("127.0.0.1"), 8888));
		try {
			/*
			std::string msg;
			cl.set_callback([&msg](boost::asio::streambuf& buf){
			  std::istream in(&buf);
			  std::getline(in, msg);
			  std::cout << msg << std::endl;
			});
			 */
			cl.run();
			io.run();
		}
		catch (const boost::system::system_error &e) {
			std::cerr << e.what() << std::endl;
		}
	};
	pool.add_task(func);
	pool.add_task(func);
	pool.add_task(func);
	pool.add_task(func);
	pool.start();
	pool.wait();
}
