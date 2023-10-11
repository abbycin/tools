/*********************************************************
	  File Name:typelist.cpp
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Sun 25 Jun 2017 4:36:51 PM CST
**********************************************************/

#include <iostream>

namespace nm
{
namespace meta
{
	template<typename...>
	struct TypeList { };
	template<>
	struct TypeList<> { };

	template<typename>
	struct Len;
	template<typename... Args>
	struct Len<TypeList<Args...>> {
		constexpr static int value = sizeof...(Args);
	};

	template<typename, typename>
	struct Append;
	template<typename... L, typename... R>
	struct Append<TypeList<L...>, TypeList<R...>> {
		using type = TypeList<L..., R...>;
	};
	template<typename T, typename... Args>
	struct Append<T, TypeList<Args...>> {
		using type = TypeList<T, Args...>;
	};
	template<typename T, typename... Args>
	struct Append<TypeList<Args...>, T> {
		using type = TypeList<Args..., T>;
	};

	template<typename, typename...>
	struct Remove;
	template<typename T>
	struct Remove<T, TypeList<>> {
		using type = TypeList<>;
	};
	template<typename T, typename... U>
	struct Remove<T, TypeList<T, U...>> {
		using type = TypeList<U...>;
	};
	template<typename T, typename Head, typename... Args>
	struct Remove<T, TypeList<Head, Args...>> {
		using type = typename Append<
			Head,
			typename Remove<T, TypeList<Args...>>::type>::type;
	};

	template<typename, typename...>
	struct RemoveAll;
	template<typename T>
	struct RemoveAll<T, TypeList<>> {
		using type = TypeList<>;
	};
	template<typename T, typename... U>
	struct RemoveAll<T, TypeList<T, U...>> {
		using type = typename RemoveAll<T, TypeList<U...>>::type;
	};
	template<typename T, typename Head, typename... Args>
	struct RemoveAll<T, TypeList<Head, Args...>> {
		using type = typename Append<
			Head,
			typename RemoveAll<T, TypeList<Args...>>::type>::type;
	};

	template<int I, typename TL>
	struct TypeAt;
	template<typename T, typename... U>
	struct TypeAt<0, TypeList<T, U...>> {
		using type = T;
	};
	template<int I, typename T, typename... Args>
	struct TypeAt<I, TypeList<T, Args...>> {
		using type = typename TypeAt<I - 1, TypeList<Args...>>::type;
	};

	template<typename, typename...>
	struct IsContain;
	template<typename T>
	struct IsContain<T, TypeList<>> {
		constexpr static bool value = false;
	};
	template<typename T, typename... U>
	struct IsContain<T, TypeList<T, U...>> {
		constexpr static bool value = true;
	};
	template<typename T, typename Head, typename... Args>
	struct IsContain<T, TypeList<Head, Args...>> {
		constexpr static bool value =
			IsContain<T, TypeList<Args...>>::value;
	};

	template<typename, typename>
	struct IndexOf;
	template<typename T>
	struct IndexOf<T, TypeList<>> {
		constexpr static int value = -1;
	};
	template<typename T, typename... U>
	struct IndexOf<T, TypeList<T, U...>> {
		constexpr static int value = 0;
	};
	template<typename T, typename Head, typename... Args>
	struct IndexOf<T, TypeList<Head, Args...>> {
	private:
		constexpr static bool have_t =
			IsContain<T, TypeList<Head, Args...>>::value;

	public:
		constexpr static int value =
			have_t ? 1 + IndexOf<T, TypeList<Args...>>::value : -1;
	};

	template<typename New, typename Old, typename TL>
	struct Replace;
	template<typename T, typename U>
	struct Replace<T, U, TypeList<>> {
		using type = TypeList<>;
	};
	template<typename T, typename U, typename... Args>
	struct Replace<T, U, TypeList<U, Args...>> {
		using type = TypeList<T, Args...>;
	};
	template<typename T, typename U, typename Head, typename... Args>
	struct Replace<T, U, TypeList<Head, Args...>> {
		using type = typename Append<
			Head,
			typename Replace<T, U, TypeList<Args...>>::type>::type;
	};

	template<typename New, typename Old, typename TL>
	struct ReplaceAll;
	template<typename T, typename U>
	struct ReplaceAll<T, U, TypeList<>> {
		using type = TypeList<>;
	};
	template<typename T, typename U, typename... Args>
	struct ReplaceAll<T, U, TypeList<U, Args...>> {
		using type =
			typename ReplaceAll<T, U, TypeList<T, Args...>>::type;
	};
	template<typename T, typename U, typename Head, typename... Args>
	struct ReplaceAll<T, U, TypeList<Head, Args...>> {
		using type = typename Append<
			Head,
			typename ReplaceAll<T, U, TypeList<Args...>>::type>::
			type;
	};

	template<typename T, typename...>
	struct Reverse;
	template<>
	struct Reverse<TypeList<>> {
		using type = TypeList<>;
	};
	template<typename T, typename... Args>
	struct Reverse<TypeList<T, Args...>> {
		using type = typename Append<
			typename Reverse<TypeList<Args...>>::type,
			T>::type;
	};

	template<typename...>
	struct PopFront;
	template<typename T>
	struct PopFront<TypeList<T>> {
		using type = TypeList<>;
	};
	template<typename T, typename... Args>
	struct PopFront<TypeList<T, Args...>> {
		using type = TypeList<Args...>;
	};

	template<typename...>
	struct PopBack;
	template<typename T>
	struct PopBack<TypeList<T>> {
		using type = TypeList<>;
	};
	template<typename... Args>
	struct PopBack<TypeList<Args...>> {
		using type = typename Reverse<typename PopFront<
			typename Reverse<TypeList<Args...>>::type>::type>::type;
	};

	template<typename...>
	struct Unique;
	template<>
	struct Unique<TypeList<>> {
		using type = TypeList<>;
	};
	template<typename T, typename... Args>
	struct Unique<TypeList<T, Args...>> {
		using type = typename Append<
			TypeList<T>,
			typename Remove<T,
					typename Unique<TypeList<Args...>>::
						type>::type>::type;
	};

	template<size_t I, typename TL>
	struct for_each_helper;
	template<size_t I, typename... Args>
	struct for_each_helper<I, TypeList<Args...>> {
		template<typename F>
		constexpr static void call(F f)
		{
			f(typename TypeAt<I - 1, TypeList<Args...>>::type {});
			for_each_helper<I - 1, TypeList<Args...>>::call(f);
		}
	};
	template<>
	struct for_each_helper<0, TypeList<>> {
		template<typename F>
		constexpr static void call(F)
		{
		}
	};
	template<typename... Args>
	struct for_each_helper<0, TypeList<Args...>> {
		template<typename F>
		constexpr static void call(F)
		{
		}
	};

	template<typename TL>
	struct ForEach1;
	template<typename... Args>
	struct ForEach1<TypeList<Args...>> {
	private:
		constexpr static size_t size = sizeof...(Args);

	public:
		template<typename F>
		constexpr static void call(F f)
		{
			for_each_helper<size,
					typename Reverse<TypeList<Args...>>::
						type>::call(f);
		}
	};

	template<typename TL>
	struct ForEach2;
	template<typename T, typename... Args>
	struct ForEach2<TypeList<T, Args...>> {
		template<typename F>
		constexpr static void call(F f)
		{
			f(T {});
			ForEach2<TypeList<Args...>>::call(f);
		}
	};
	template<>
	struct ForEach2<TypeList<>> {
		template<typename F>
		constexpr static void call(F)
		{
		}
	};

	template<typename... Args>
	struct overloaded : Args... {
		using Args::operator()...;
	};

	template<typename... Args>
	overloaded(Args...) -> overloaded<Args...>;
}
}

template<typename T, typename U>
struct is_same {
	constexpr static bool value = false;
};
template<typename T>
struct is_same<T, T> {
	constexpr static bool value = true;
};

int main()
{
	using namespace nm::meta;
	// Len
	static_assert(Len<TypeList<int, long>>::value == 2);
	// Append
	using l0 = typename Append<int, TypeList<long, char, char>>::type;
	static_assert(is_same<l0, TypeList<int, long, char, char>>::value);
	// Remove
	using l1 = typename Remove<char, l0>::type;
	static_assert(is_same<l1, TypeList<int, long, char>>::value);
	// RemoveAll
	using l2 = typename RemoveAll<char, l0>::type;
	static_assert(is_same<l2, TypeList<int, long>>::value);
	// TypeAt
	static_assert(is_same<typename TypeAt<Len<l0>::value - 1, l0>::type,
			      char>::value);
	// IsContain
	static_assert(!IsContain<double, l0>::value);
	// IndexOf
	static_assert(IndexOf<int, TypeList<>>::value == -1);
	static_assert(IndexOf<int, TypeList<int>>::value == 0);
	static_assert(IndexOf<char, TypeList<char, int>>::value == 0);
	static_assert(IndexOf<int, TypeList<char, int>>::value == 1);
	static_assert(IndexOf<int, TypeList<long, char, int>>::value == 2);
	static_assert(IndexOf<double, l0>::value == -1);
	static_assert(IndexOf<int, l0>::value == 0);
	static_assert(IndexOf<long, l0>::value == 1);
	static_assert(IndexOf<char, l0>::value == 2);
	// Replace
	using l3 = typename Replace<double, char, l0>::type;
	static_assert(is_same<l3, TypeList<int, long, double, char>>::value);
	// ReplaceAll
	using l4 = typename ReplaceAll<double, char, l0>::type;
	static_assert(is_same<l4, TypeList<int, long, double, double>>::value);
	// Reverse
	using l5 = typename Reverse<l4>::type;
	static_assert(is_same<l5, TypeList<double, double, long, int>>::value);
	// PopFront
	static_assert(is_same<typename PopFront<l5>::type,
			      TypeList<double, long, int>>::value);
	// PopBack
	static_assert(is_same<typename PopBack<l5>::type,
			      TypeList<double, double, long>>::value);
	// Unique
	static_assert(is_same<typename Unique<l0>::type,
			      TypeList<int, long, char>>::value);
	// ForEach
	using l6 = TypeList<int, long, double, const char *>;
	using std::cout;

	auto vi = overloaded { [](int) { cout << "int\n"; },
			       [](double) { cout << "double\n"; },
			       [](long) { cout << "long\n"; },
			       [](auto) { cout << "other\n"; } };
	ForEach1<l6>::call(vi);
	cout << '\n';
	ForEach2<l6>::call(vi);
}
