/*********************************************************
	  File Name: functor_map.h
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sat 07 Apr 2018 03:43:56 PM CST
**********************************************************/

#ifndef FUNCTOR_MAP_H_
#define FUNCTOR_MAP_H_

#include <stdexcept>
#include <functional>
#include <map>
#include <mutex>
#include <string>

namespace nm
{
namespace meta
{
	template<typename...>
	struct test_arg;
	template<>
	struct test_arg<> {
		constexpr static bool value = true;
	};
	template<typename T>
	struct test_arg<T> {
		constexpr static bool value = !std::is_reference_v<T> &&
			std::is_copy_assignable_v<T> &&
			std::is_constructible_v<T>;
	};
	template<typename T, typename... Ts>
	struct test_arg<T, Ts...> {
		constexpr static bool value =
			test_arg<T>::value && test_arg<Ts...>::value;
	};

	template<typename>
	struct Inspector;
	template<typename R, typename... Args>
	struct Inspector<R(Args...)> {
		constexpr static bool is_r_valid = std::is_void_v<R> ||
			(std::is_copy_assignable_v<R> &&
			 std::is_constructible_v<R>);
		constexpr static bool is_arg_valid = test_arg<Args...>::value;
		static_assert(is_r_valid,
			      "invalid return type, must be `void` or can be "
			      "copied and constructed");
		static_assert(is_arg_valid,
			      "invalid argument type, must be copyable and "
			      "can't be reference");
		using arg_t = std::tuple<Args...>;
		using res_t = R;
	};
	template<typename R, typename... Args>
	struct Inspector<R (*)(Args...)> : Inspector<R(Args...)> { };
	template<typename R, typename... Args>
	struct Inspector<R (&)(Args...)> : Inspector<R(Args...)> { };
	template<typename R, typename Object, typename... Args>
	struct Inspector<R (Object::*)(Args...)> : Inspector<R(Args...)> { };
	template<typename R, typename Object, typename... Args>
	struct Inspector<R (Object::*)(Args...) const> : Inspector<R(Args...)> {
	};
	template<typename R, typename Object, typename... Args>
	struct Inspector<R (Object::*)(Args...) volatile>
		: Inspector<R(Args...)> { };
	template<typename R, typename Object, typename... Args>
	struct Inspector<R (Object::*)(Args...) const volatile>
		: Inspector<R(Args...)> { };

	// functor like
	template<typename Lambda>
	struct Inspector : Inspector<decltype(&Lambda::operator())> { };
	template<typename Lambda>
	struct Inspector<Lambda &> : Inspector<decltype(&Lambda::operator())> {
	};
	template<typename Lambda>
	struct Inspector<Lambda &&> : Inspector<Lambda &> { };

	template<typename R>
	struct call_impl {
		template<typename F, typename Arg>
		static R call(F &&f, Arg &args)
		{
			R res;
			std::forward<F>(f)(&args, &res);
			return res;
		}

		template<typename F, typename TupleType, size_t... Is>
		static void invoke(R *res,
				   F &&f,
				   TupleType &args,
				   std::index_sequence<Is...>)
		{
			*res = std::forward<F>(f)(
				std::forward<std::tuple_element_t<
					Is,
					std::remove_reference_t<TupleType>>>(
					std::get<Is>(args))...);
		}
	};
	template<>
	struct call_impl<void> {
		template<typename F, typename Arg>
		static void call(F &&f, Arg &args)
		{
			std::forward<F>(f)(&args, nullptr);
		}

		template<typename F, typename TupleType, size_t... Is>
		static void invoke(void *,
				   F &&f,
				   TupleType &args,
				   std::index_sequence<Is...>)
		{
			std::forward<F>(f)(
				std::forward<std::tuple_element_t<
					Is,
					std::remove_reference_t<TupleType>>>(
					std::get<Is>(args))...);
		}
	};
}

class FunctorMap {
public:
	using Fp = std::function<void(void *, void *)>;

	FunctorMap() = default;

	~FunctorMap()
	{
		this->clear();
	}

	FunctorMap(const FunctorMap &) = delete;
	FunctorMap &operator=(const FunctorMap &) = delete;

	FunctorMap(FunctorMap &&rhs) noexcept
		: functors_ { std::move(rhs.functors_) }
		, tidy_ { std::move(rhs.tidy_) }
	{
	}

	FunctorMap &operator=(FunctorMap &&rhs) noexcept
	{
		if (this != &rhs) {
			this->~FunctorMap();
			new (this) FunctorMap(std::move(rhs));
		}
		return *this;
	}

	template<typename F>
	bool bind(const std::string &name, F &&f)
	{
		if (functors_.count(name) > 0) {
			return false;
		}
		using inspector = meta::Inspector<F>;
		using arg_t = typename inspector::arg_t;
		using res_t = typename inspector::res_t;
		auto fp = [f = std::forward<F>(f)](void *args, void *res)
		{
			using indices = std::make_index_sequence<
				std::tuple_size<arg_t>::value>;
			arg_t &arg = *static_cast<arg_t *>(args);
			res_t *tmp = static_cast<res_t *>(res);
			meta::call_impl<res_t>::invoke(tmp, f, arg, indices {});
		};
		functors_[name] = std::move(fp);
		std::lock_guard<std::mutex> lg { mtx_ };
		// use res_t* for void type
		types_<res_t *, arg_t>.insert({ name, nullptr });
		tidy_[name] = [name] { types_<res_t *, arg_t>.erase(name); };
		return true;
	}

	template<typename R = void, typename... Args>
	R call(const std::string &name, Args &&...args)
	{
		auto params = std::make_tuple(std::forward<Args>(args)...);
		auto iter = functors_.find(name);
		if (iter == functors_.end()) {
			return R();
		}
		if (types_<R *, std::tuple<Args...>>.count(name) <= 0) {
			throw std::runtime_error("parameter mismatch");
		}
		return meta::call_impl<R>::call(iter->second, params);
	}

	bool contain(const std::string &name)
	{
		return functors_.count(name) > 0;
	}

	bool remove(const std::string &name)
	{
		if (!this->contain(name)) {
			return false;
		}
		functors_.erase(name);
		std::lock_guard<std::mutex> lg { mtx_ };
		auto iter = tidy_.find(name);
		iter->second();
		tidy_.erase(iter);
		return true;
	}

	void clear()
	{
		functors_.clear();
		std::lock_guard<std::mutex> lg { mtx_ };
		for (auto &c : tidy_) {
			c.second();
		}
		tidy_.clear();
	}

private:
	std::map<std::string, Fp> functors_;
	std::map<std::string, std::function<void()>> tidy_;
	static inline std::mutex mtx_;
	// vs2017 bug:
	// https://developercommunity.visualstudio.com/content/problem/261624/multiple-initializations-of-inline-static-data-mem.html
	template<typename R, typename... Args>
	static inline std::map<std::string, std::tuple<R, Args...> *> types_ {};
};
}

#endif
