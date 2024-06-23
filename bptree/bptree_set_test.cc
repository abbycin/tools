// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2024-06-22 14:56:21
 */

#include "bptree_set.h"

int main()
{
	nm::BpTreeSet<int, 5> s { 1, 2, 3, 4, -1 };

	auto it = s.range(-4, 0);

	while (it) {
		printf("%d\n", it.data());
		++it;
	}

	assert(s.get(-1) != nullptr);
}