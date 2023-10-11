/*********************************************************
	  File Name: function_traits.h
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sat 11 Jun 2018 18:46:33 PM CST
**********************************************************/

#ifndef FUNCTION_TRAITS_H_
#define FUNCTION_TRAITS_H_

#include <tuple>
#include <functional>
#include <type_traits>

namespace nm
{
namespace meta
{
	template<typename>
	struct Inspector;
	template<typename R, typename... Args>
	struct Inspector<R(Args...)> {
		using arg_t = std::tuple<std::remove_reference_t<Args>...>;
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
}
}

#endif
