// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2024-06-23 18:42:52
 */

#include "bptree_map.h"

int main()
{
	nm::BpTreeMap<int, int> m {};

	m[1] = 1;
	m[2] = 2;

	auto it = m.range(-1, 3);

	while (it) {
		printf("%d => %d\n", it.data().first, it.data().second);
		++it;
	}
}