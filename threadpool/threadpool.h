/*********************************************************
	  File Name:threadpool.h
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Thu 04 Aug 2016 09:33:30 AM CST
**********************************************************/

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <cassert>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <type_traits>
#include <vector>

namespace nm
{
class threadpool final {
public:
	threadpool() : threadpool { std::launch::async }
	{
	}
	threadpool(std::launch p,
		   std::size_t threads = std::thread::hardware_concurrency())
		: policy_(p)
		, is_exit_(false)
		, is_stop_(false)
		, is_start_(policy_ == std::launch::async ? true : false)
		, task_limit_(1000)
	{
		for (; threads > 0; --threads) {
			auto worker = [&]
			{
				for (;;) {
					std::unique_lock<std::mutex> l(
						queue_lock_);
					queue_cond_.wait(
						l,
						[this]
						{
							if (!is_start_)
								return false;
							return is_stop_ ||
								is_exit_ ||
								!tasks_.empty();
						});
					// caller request to stop all
					// task.
					if (is_stop_)
						return;
					// normal exit, when pool is
					// going to destroyed.
					if (tasks_.empty()) {
						if (is_exit_)
							break;
					} else {
						auto task(std::move(
							tasks_.front()));
						tasks_.pop();
						if (tasks_.empty())
							wait_cond_.notify_one();
						l.unlock();
						try {
							task();
						}
						catch (std::exception &e) {
							// abort program
							// when
							// encounter an
							// exception.
							assert(false);
						}
					}
				}
			};
			workers_.emplace_back(worker);
		}
	}
	threadpool(threadpool &&) = delete;
	threadpool(const threadpool &) = delete;
	threadpool &operator=(const threadpool &) = delete;
	~threadpool() noexcept
	{
		try {
			std::unique_lock<std::mutex> l(queue_lock_);
			is_start_ = true;
			is_exit_ = true;
			queue_cond_.notify_all();
		}
		catch (std::system_error &) {
			assert(false);
		}
		for (auto &t : workers_) {
			try {
				t.join();
			}
			catch (std::system_error &) {
				// do nothing.
			}
		}
	}
	template<typename F, typename... Args>
	auto add_task(F &&f, Args &&...args)
		-> std::future<typename std::invoke_result<F, Args...>::type>
	{
		using R = typename std::invoke_result<F, Args...>::type;
		auto task = std::make_shared<std::packaged_task<R()>>(std::bind(
			std::forward<F>(f), std::forward<Args>(args)...));
		auto res(task->get_future());
		std::unique_lock<std::mutex> l(queue_lock_);
		if (tasks_.size() >= task_limit_)
			throw std::runtime_error("task queue is full.");
		check_status_(__func__);
		tasks_.emplace([task] { (*task)(); });
		if (policy_ == std::launch::async)
			queue_cond_.notify_one();
		return res; // NRVO
	}
	size_t queue_size_limit()
	{
		std::unique_lock<std::mutex> l(queue_lock_);
		check_status_(__func__);
		return task_limit_;
	}
	void set_queue_size_limit(size_t size)
	{
		std::unique_lock<std::mutex> l(queue_lock_);
		check_status_(__func__);
		auto old_limit = task_limit_;
		task_limit_ = (size > 1 ? size : 1);
		// if new limit less then old one,then wake up all workers_ to
		// process as many tasks_ as they can.
		if (task_limit_ < old_limit)
			queue_cond_.notify_all();
	}
	void start()
	{
		std::unique_lock<std::mutex> l(queue_lock_);
		check_status_(__func__);
		is_start_ = true;
		queue_cond_.notify_all();
	}
	void wait()
	{
		std::unique_lock<std::mutex> l(queue_lock_);
		check_status_(__func__);
		wait_cond_.wait(l, [&] { return tasks_.empty(); });
	}
	void pause()
	{
		std::unique_lock<std::mutex> l(queue_lock_);
		check_status_(__func__);
		is_start_ = false;
	}
	void stop()
	{
		std::unique_lock<std::mutex> l(queue_lock_);
		check_status_(__func__);
		is_stop_ = true;
		while (!tasks_.empty())
			tasks_.pop();
		queue_cond_.notify_all();
	}
	bool valid() const
	{
		std::unique_lock<std::mutex> l(queue_lock_);
		return !is_stop_;
	}

private:
	std::launch policy_;
	// we don't use `atomic` variable here.
	bool is_exit_;
	bool is_stop_;
	bool is_start_;
	size_t task_limit_;
	std::vector<std::thread> workers_;
	std::queue<std::function<void()>> tasks_;
	mutable std::mutex queue_lock_;
	std::condition_variable queue_cond_, wait_cond_;
	void check_status_(const std::string &func)
	{
		if (is_stop_)
			throw std::runtime_error(func +
						 " on stopped threadpool.");
	}
};

}

#endif
