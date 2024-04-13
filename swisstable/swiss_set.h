// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-10-17 11:11:16
 */

#ifndef SWISS_SET_20231017111116
#define SWISS_SET_20231017111116

#include "swisstable.h"
#include <initializer_list>

namespace nm
{
template<typename T>
class SetPolicy {
public:
	static_assert(!std::is_reference_v<T>, "forbid reference as key");
	using key_type = std::remove_cvref_t<T>;
	using value_type = std::remove_cvref_t<T>;

	static const key_type &key(const value_type &v)
	{
		static_assert(std::is_same_v<key_type, value_type>);
		return v;
	}
};

template<typename Key, typename Hash = SwissHash, typename Eq = SwissEq>
class SwissSet : public detail::Swiss<SetPolicy<Key>, Hash, Eq> {
	using Base = detail::Swiss<SetPolicy<Key>, Hash, Eq>;

public:
	using Base::Base;
	using Base::begin;
	using Base::end;
	using Base::insert;
	using Base::emplace;
	using Base::erase;
	using Base::reserve;
	using Base::find;
	using iterator = Base::iterator;

	SwissSet(std::initializer_list<typename Base::value_type> il) : Base {}
	{
		for (auto &&x : il)
			insert(x);
	}
};
}

#endif // SWISS_SET_20231017111116
