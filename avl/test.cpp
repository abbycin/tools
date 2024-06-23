// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-07-15 18:42:48
 */

#include <iostream>
#include <random>
#include <cassert>
#include "avl.h"

int main()
{
	nm::AVL<int> avl {};
	std::random_device rd {};
	std::mt19937 mt { rd() };
	std::uniform_int_distribution<int> dist { -10, 53 };

	for (int i = 0; i < 53; ++i)
		avl.emplace(dist(mt));

	auto exists = avl.find(1);
	auto ok = avl.emplace(1);

	assert((exists && !ok) || (!exists && ok));
	std::cout << std::boolalpha;
	std::cout << "is avl? " << avl.is_avl() << '\n';
	std::cout << "size? " << avl.size() << '\n';

	std::cout << "remove 10 ok? " << avl.remove(10) << '\n';
	std::cout << "size? " << avl.size() << '\n';
	avl.draw_tree("/tmp/x.dot");
}
