// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-10-16 13:23:04
 */

#ifndef SWISS_MAP_H_20231016132304
#define SWISS_MAP_H_20231016132304

#include "swisstable.h"
#include <initializer_list>

namespace nm
{
template<typename K, typename V>
class MapPolicy {
public:
	static_assert(!std::is_reference_v<K>, "forbid reference as key");
	using key_type = std::remove_cvref_t<K>;
	using value_type = std::pair<key_type, V>;

	static const key_type &key(const value_type &v)
	{
		return v.first;
	}
};

template<typename Key,
	 typename Val,
	 typename Hash = SwissHash,
	 typename Eq = SwissEq>
class SwissMap : public detail::Swiss<MapPolicy<Key, Val>, Hash, Eq> {
	using Base = detail::Swiss<MapPolicy<Key, Val>, Hash, Eq>;

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

	SwissMap(std::initializer_list<typename Base::value_type> il) : Base {}
	{
		for (auto &x : il)
			insert(x);
	}

	Val &operator[](const Key &k)
	{
		auto it = find(k);
		if (it == end())
			it = emplace(k, Val {});
		return it->second;
	}
};
}

#endif // SWISS_MAP_H_20231016132304
