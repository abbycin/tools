// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-10-16 19:51:17
 */

#include "swiss_map.h"

int main()
{
	{
		nm::SwissMap<int, int> m { { 5, 6 } };

		m.emplace(1, 2);
		m.emplace(2, 3);
		m.emplace(3, 4);

		for (auto [k, v] : m)
			printf("%d => %d\n", k, v);
		auto it = m.find(1);
		printf("=> %d %d\n", it->first, it->second);

		m.erase(1);
		printf("contains 1? %s\n", m.contains(1) ? "yes" : "no");
		for (auto [k, v] : m)
			printf("%d => %d\n", k, v);

		printf("before clear load_factor %f\n", m.load_factor());
		m.clear();
		for (auto [k, v] : m)
			printf("%d => %d\n", k, v);
		printf("after clear load_factor %f\n", m.load_factor());
	}

	{
		using namespace std::string_literals;
		nm::SwissMap<std::string, int> m { { "e"s, 4 } };

		m.emplace("a"s, 1);
		m.emplace("b"s, 2);
		m.emplace("c"s, 3);

		m.insert({ "d"s, 4 });

		auto it = m.find("c"s);
		auto moha = "moha"s;

		it->second = 666;

		m[moha] = 1926;
		printf("moha => %d\n", m[moha]);

		m[moha] = 233;

		for (auto &[k, v] : m)
			printf("%s => %d\n", k.c_str(), v);
	}
}