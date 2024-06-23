// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2024-06-22 12:37:33
 */

#ifndef BPTREE_MAP_1719031053_H_
#define BPTREE_MAP_1719031053_H_

#include "bptree.h"

namespace nm
{
namespace detail
{
	template<typename Key, typename Val>
	struct BpTreeMapPolicy {
		static_assert(!std::is_reference_v<Key>,
			      "forbid reference as key");
		using key_type = std::remove_cvref_t<Key>;
		using value_type =
			std::pair<key_type, std::remove_cvref_t<Val>>;

		static const key_type &key(const value_type &v)
		{
			return v.first;
		}
	};
}

template<typename Key, typename Val, int M = 3>
class BpTreeMap : public BpTree<detail::BpTreeMapPolicy<Key, Val>, M> {
	using Base = BpTree<detail::BpTreeMapPolicy<Key, Val>, M>;

public:
	using Base::Base;
	using Base::put;
	using Base::get;
	using Base::del;
	using Base::range;
	using Base::clear;

	using iter = typename Base::iter;

	BpTreeMap(std::initializer_list<typename Base::val_t> il) : Base {}
	{
		for (auto &&x : il)
			put(x);
	}

	std::remove_cvref_t<Val> &operator[](const typename Base::key_t &k)
	{
		auto it = get(k);
		if (!it) {
			put({ k, std::remove_cvref_t<Val> {} });
			it = get(k);
			assert(it);
		}
		return it->second;
	}
};
}

#endif // BPTREE_MAP_1719031053_H_
