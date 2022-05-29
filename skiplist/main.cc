// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2022-05-24 21:35:52
 */

#include "skiplist.h"
#include <iostream>

int main()
{
	using nm::SkipList;
	SkipList<int, int> sl {};

	for (int i = 0; i < 10; ++i)
		sl.insert(i, rand() % 20);

	std::cout << std::boolalpha;
	std::cout << "contains key 4?" << sl.contains(4) << '\n';
	std::cout << "rm exist key ok? " << sl.remove(4) << '\n';
	std::cout << "size " << sl.size() << '\n';
	std::cout << "rm none exist key ok? " << sl.remove(233) << '\n';
	std::cout << "size " << sl.size() << '\n';

	std::cout << "iterate\n";
	for (auto& x : sl)
		std::cout << x.first << " : " << x.second << '\n';

	std::cout << "range\n";
	auto [b, e] = sl.range(4, 6);
	while (b != e) {
		printf("%d => %d\n", b.key(), b.val());
		++b;
	}
}