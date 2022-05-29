/*********************************************************
	  File Name:signal.h
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sun 17 Jul 2016 07:34:00 PM CST
**********************************************************/

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <utility>
#include <list>

namespace nm
{
namespace
{
	template<typename>
	struct sig_trait;
	template<typename Res, typename... Args>
	struct sig_trait<Res(Args...)> {
		using mFunc = Res (*)(void *, Args...);
		using Func = Res (*)(Args...);
		sig_trait(const sig_trait &) = delete;
		sig_trait &operator=(const sig_trait &) = delete;
		sig_trait(sig_trait &&rhs)
		{
			obj = rhs.obj;
			callable = rhs.callable;
			mcallable = rhs.mcallable;
		}
		sig_trait(void *o, mFunc fp)
		{
			obj = o;
			mcallable = fp;
			callable = nullptr;
		}
		sig_trait(void *o, Func fp)
		{
			obj = o;
			callable = fp;
			mcallable = nullptr;
		}
		template<typename Obj, Res (Obj::*mfp)(Args...)>
		static sig_trait connect(Obj *o)
		{
			return { o, [](void *obj, Args... args) {
					return (static_cast<Obj *>(obj)->*mfp)(
						std::forward<Args>(args)...);
				} };
		}
		template<typename F>
		static sig_trait connect(F &&fp)
		{
			return { nullptr, fp };
		}
		void *obj;
		mFunc mcallable;
		Func callable;
		template<typename... Paras>
		Res call(Paras &&...paras) const
		{
			if (obj)
				return mcallable(obj,
						 std::forward<Paras>(paras)...);
			return callable(std::forward<Paras>(paras)...);
		}
	};
}
namespace signal
{
	namespace
	{
		using std::list;
	}
	template<typename>
	class Signal;
	template<typename Res, typename... Paras>
	class Signal<Res(Paras...)> {
	public:
		using data_type = Res(Paras...);
		using handle_type =
			typename list<sig_trait<data_type>>::iterator;
		Signal()
		{
		}
		Signal(const Signal &) = delete;
		Signal &operator=(const Signal &) = delete;
		~Signal()
		{
		}
		template<typename Obj, Res (Obj::*mfp)(Paras...)>
		handle_type &connect(Obj *o)
		{ // sig_trait<data_type>::template connect<Obj, mfp>(o); RVO
		  // optimization
			objs.push_back(
				sig_trait<data_type>::template connect<Obj,
								       mfp>(o));
			return --objs.end();
		}
		template<typename F>
		handle_type &connect(F &&fp)
		{ // sig_trait<data_type>::connect(fp); RVO optimization
			objs.push_back(sig_trait<data_type>::connect(fp));
			return --objs.end();
		}
		void disconnect(handle_type &handle)
		{
			objs.erase(handle);
		}
		template<typename F>
		void disconnect(F &&fp)
		{
			for (auto iter = objs.begin(); iter != objs.end();) {
				if (fp == iter->callable)
					iter = objs.erase(iter);
				else
					++iter;
			}
		}
		template<typename... Args>
		void emit(Args &&...args) const
		{
			for (auto &x : objs)
				x.call(std::forward<Args>(args)...);
		}
		std::size_t size() const
		{
			return objs.size();
		}
		bool empty() const
		{
			return objs.empty();
		}

	private:
		list<sig_trait<data_type>> objs;
	};
}
}

#endif
