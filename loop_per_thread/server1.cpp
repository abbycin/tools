/***********************************************
	File Name: server1.cpp
	Author: Abby Cin
	Mail: abbytsing@gmail.com
	Created Time: 9/11/16 2:29 PM
***********************************************/

#include <iostream>
#include <thread>
#include <boost/asio.hpp>

class Service : public std::enable_shared_from_this<Service> {
public:
	Service(const Service &) = delete;
	Service &operator=(const Service &) = delete;
	Service(std::shared_ptr<boost::asio::ip::tcp::socket> sock)
		: socket_(sock), msg_("pong\n")
	{
	}

	static std::shared_ptr<Service>
	make_service(std::shared_ptr<boost::asio::ip::tcp::socket> sock)
	{
		return std::make_shared<Service>(sock);
	}

	void run()
	{
		do_read();
	}

private:
	std::shared_ptr<boost::asio::ip::tcp::socket> socket_;
	std::string msg_;
	boost::asio::streambuf buf_;

	void do_read()
	{
		auto self(shared_from_this());
		boost::asio::async_read_until(
			*socket_,
			buf_,
			'\n',
			[this, self](const boost::system::error_code &ec,
				     size_t bytes) { on_read(ec, bytes); });
	}

	void on_read(const boost::system::error_code &ec, size_t)
	{
		if (ec) {
			; // std::cerr << "--->>> " << __LINE__ << "\t" <<
			  // ec.message() << std::endl;
		} else {
			do_write();
		}
	}

	void do_write()
	{
		auto self(shared_from_this());
		boost::asio::async_write(
			*socket_,
			boost::asio::buffer(msg_),
			[this, self](const boost::system::error_code &ec,
				     size_t bytes) { on_write(ec, bytes); });
	}

	void on_write(const boost::system::error_code &ec, size_t)
	{
		if (ec) {
			// std::cerr << "--->>> " << __LINE__ << "\t" <<
			// ec.message() << std::endl;
			return;
		}
		// no more read.
		do_read();
	}
};

class Acceptor {
public:
	Acceptor(boost::asio::io_service *io,
		 boost::asio::ip::tcp::endpoint &&ep)
		: io_(io), acceptor_(*io_, std::move(ep), true), is_quit_(false)
	{
		std::atomic_init(&is_quit_, false);
	}

	Acceptor(const Acceptor &) = delete;
	Acceptor &operator=(const Acceptor &) = delete;

	void start()
	{
		acceptor_.listen(SOMAXCONN); // default SOMAXCONN is 128
		start_accept();
	}

	void stop()
	{
		is_quit_ = true;
	}

private:
	boost::asio::io_service *io_;
	boost::asio::ip::tcp::acceptor acceptor_;
	std::atomic<bool> is_quit_;

	void start_accept()
	{
		auto sock(std::make_shared<boost::asio::ip::tcp::socket>(*io_));
		acceptor_.async_accept(
			*sock,
			[this, sock](const boost::system::error_code &ec)
			{ handle_accept(ec, sock); });
	}

	void handle_accept(const boost::system::error_code &ec,
			   std::shared_ptr<boost::asio::ip::tcp::socket> sock)
	{
		if (!ec) {
			Service::make_service(sock)->run();
		} else {
			// std::cerr << "--->>> " << __LINE__ << "\t" <<
			// ec.message() << std::endl;
		}
		if (!is_quit_)
			start_accept();
		else
			acceptor_.close();
	}
};

class Server {
public:
	Server(boost::asio::ip::tcp::endpoint &&ep)
		: work_(new boost::asio::io_service::work(io_))
		, acceptor_(new Acceptor(&io_, std::move(ep)))
	{
	}

	void start(int num_threads)
	{
		assert(num_threads > 0);
		acceptor_->start();

		for (int i = 0; i < num_threads; ++i) {
			std::unique_ptr<std::thread> th(
				new std::thread([this] { io_.run(); }));
			threads_.push_back(std::move(th));
		}
	}

	void stop()
	{
		acceptor_->stop();
		io_.stop();
		for (auto &x : threads_) {
			if (x->joinable())
				x->join();
		}
	}

private:
	boost::asio::io_service io_;
	/**
	 * @code io_service::run @endcode will return as soon as there is no
	 * more work to do, so consider an application that have some producer
	 * and consumer threads, producers occasionally produce works and post
	 * them to consumer threads with io_service::post, but if all works
	 * finished, then
	 * @code io_service::run @endcode will return and the consumer thread
	 * will be stopped, so we need an arbitrary work to keep io_service
	 * busy, in this case we will use @code io_service::work @endcode
	 * directly.
	 *
	 * BTW, we don't need to destroy @code io_service::work @endcode
	 * explicitly by @code shared_ptr<io_service>::reset() @encode, we will
	 * use
	 * @code io_service::stop() @endcode
	 */
	std::unique_ptr<boost::asio::io_service::work> work_;
	std::unique_ptr<Acceptor> acceptor_;
	std::vector<std::unique_ptr<std::thread>> threads_;
};

int main()
{
	try {
		Server se(boost::asio::ip::tcp::endpoint(
			boost::asio::ip::address_v4::any(), 8888));
		se.start(4);
		std::this_thread::sleep_for(std::chrono::minutes(10));
		se.stop();
	}
	catch (const boost::system::system_error &e) {
		std::cerr << e.what() << std::endl;
	}
}
