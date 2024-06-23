// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2024-06-22 12:37:23
 */

#ifndef BPTREE_SET_1719031043_H_
#define BPTREE_SET_1719031043_H_

#include "bptree.h"

namespace nm
{
namespace detail
{
	template<typename T>
	struct BpTreeSetPolicy {
		static_assert(!std::is_reference_v<T>,
			      "forbid reference as key");
		using key_type = std::remove_cvref_t<T>;
		using value_type = key_type;

		static const key_type &key(const value_type &v)
		{
			return v;
		}
	};
}

template<typename Key, int M = 3>
class BpTreeSet : public BpTree<detail::BpTreeSetPolicy<Key>, M> {
	using Base = BpTree<detail::BpTreeSetPolicy<Key>, M>;

public:
	using Base::Base;
	using Base::put;
	using Base::get;
	using Base::del;
	using Base::range;
	using Base::clear;

	using iter = typename Base::iter;

	BpTreeSet(std::initializer_list<typename Base::val_t> il) : Base {}
	{
		for (auto &&x : il)
			put(x);
	}
};
}

#endif // BPTREE_SET_1719031043_H_
