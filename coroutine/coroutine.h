/*********************************************************
	  File Name: coroutine.h
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sun 03 Jun 2018 10:24:50 AM CST
**********************************************************/

#ifndef CO_ROUTINE_H_
#define CO_ROUTINE_H_

#include <cstdlib>
#include <cstring>
#include <optional>
#include "../meta/function_traits.h"

namespace nm
{
extern "C" {
void *switch_stack(void *obj, void *context, void *arg);
}

template<typename T>
class Coroutine;

template<typename T>
void co_call_member_function(Coroutine<T> *obj, void *ctx);

template<typename T>
class Coroutine {
public:
	template<typename Type>
	friend void co_call_member_function(Coroutine<Type> *, void *);

	enum {
		Fixed_Stack_Size = 8 * 1024
	};

	class Iterator {
		template<typename Type>
		friend class Coroutine;

	public:
		~Iterator()
		{
		}

		Iterator(const Iterator &) = delete;
		Iterator &operator=(const Iterator &) = delete;

		Iterator(Iterator &&rhs) : co_ { rhs.co_ }
		{
			rhs.co_ = nullptr;
		}

		Iterator &operator=(Iterator &&rhs)
		{
			this->co_ = rhs.co_;
			rhs.co_ = nullptr;
			return *this;
		}

		bool operator==(const Iterator &rhs)
		{
			return this->co_ == rhs.co_;
		}

		bool operator!=(const Iterator &rhs)
		{
			return this->co_ != rhs.co_;
		}

		Iterator &operator++()
		{
			co_->resume();
			if (co_->done()) {
				co_ = nullptr;
			}
			return *this;
		}

		T &operator*()
		{
			return co_->data();
		}

	private:
		Coroutine<T> *co_;

		Iterator() : co_ { nullptr }
		{
		}

		Iterator(Coroutine *co) : co_ { co }
		{
		}
	};

	Coroutine() = default;

	~Coroutine()
	{
		free(stack_);
		stack_ = nullptr;
		if (deleter_) {
			deleter_(arg_);
			arg_ = nullptr;
		}
	}

	Coroutine(const Coroutine &) = delete;
	Coroutine(Coroutine &&) = delete;
	Coroutine &operator=(const Coroutine &) = delete;
	Coroutine &operator=(Coroutine &&) = delete;

	template<typename F, typename... Args>
	void start(F &&f, Args &&...args)
	{
		this->~Coroutine();
		stack_ = malloc(Fixed_Stack_Size);
		memset(stack_, 0, Fixed_Stack_Size);
		// call pop in `switch_stack` will grow rsp, here I reserve 4k
		// space for push and I don't check alignment, you can adjust by
		// using std::align since C++11
		new_sp_ = (char *)stack_ + 4096;

		this->done_ = false;
		using arg_t = typename meta::Inspector<F>::arg_t;
		arg_ = new arg_t { std::forward<Args>(args)... };
		deleter_ = [](void *data)
		{ delete static_cast<arg_t *>(data); };
		fp_ = [f = std::forward<F>(f)](void *arg)
		{
			arg_t *param = static_cast<arg_t *>(arg);
			std::apply(f, *param);
		};
		// skip 6 registers r12~r15 and rbx rbp, assign next instruction
		// to rip
		*((void **)((char *)new_sp_ + 6 * 8)) =
			(void *)co_call_member_function<T>;
		new_sp_ = switch_stack(this, new_sp_, arg_);
	}

	void yield(const T &t)
	{
		data_ = t;
		this->yield();
	}

	void yield()
	{
		new_sp_ = switch_stack(nullptr, new_sp_, nullptr);
	}

	Iterator begin()
	{
		if (new_sp_) {
			this->resume();
			if (this->done()) {
				return { nullptr };
			}
		}
		return { this };
	}

	Iterator end()
	{
		return { nullptr };
	}

private:
	void *new_sp_ { nullptr };
	void *stack_ { nullptr };
	bool done_ { false };
	std::function<void(void *)> fp_;
	std::function<void(void *)> deleter_;
	void *arg_ { nullptr };
	std::optional<T> data_ {};

	bool done()
	{
		return done_;
	}

	T &data()
	{
		return data_.value(); // let it throw
	}

	void resume()
	{
		new_sp_ = switch_stack(nullptr, new_sp_, nullptr);
	}

	void agent(void *ctx)
	{
		new_sp_ = switch_stack(nullptr, ctx, nullptr);
		fp_(arg_);
		done_ = true;
		data_.reset();
		this->yield();
	}
};

template<typename T>
void co_call_member_function(Coroutine<T> *obj, void *ctx)
{
	obj->agent(ctx);
}
}

#endif
