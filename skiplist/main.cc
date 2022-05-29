// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2022-05-24 21:35:52
 */

#include "skiplist.h"
#include <cstdlib>
#include <iostream>

int main()
{
	using nm::SkipList;
	SkipList<int, int, 4> sl {};
	srand(time(NULL));
	int limit = 16;

	for (int i = 0; i < limit; ++i)
		sl.insert(i, rand() % limit);

	std::cout << std::boolalpha;
	std::cout << "contains key 4?" << sl.contains(4) << '\n';
	std::cout << "rm exist key ok? " << sl.remove(4) << '\n';
	std::cout << "size " << sl.size() << '\n';
	std::cout << "rm none exist key ok? " << sl.remove(233) << '\n';
	std::cout << "size " << sl.size() << '\n';

	std::cout << "iterate\n";
	for (auto &x : sl)
		std::cout << x.first << " : " << x.second << '\n';

	std::cout << "range\n";
	sl.remove(6);
	auto [b, e] = sl.range(4, 8);
	while (b != e) {
		printf("%d => %d\n", b->first, b->second);
		++b;
	}

	std::cout << "dump\n";
	for (int i = sl.level(); i >= 0; --i) {
		auto b = sl.begin(i);
		std::cout << "-------- level " << i << " ---------\n";
		while (b) {
			printf("[%d, %d] ", b->first, b->second);
			++b;
		}
		std::cout << '\n';
	}
}