// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-11-02 14:50:49
 */

#include "bptree.h"
#include <exception>
#include <cstdio>
#include <unordered_set>
#include <vector>
#include <random>

struct kv_t {
	int key;
	int val;
};

struct Policy {
	using key_type = int;
	using value_type = kv_t;

	static const key_type &key(const value_type &v)
	{
		return v.key;
	}
};

void base_test()
{
	nm::BpTree<Policy, 64> t {};
	int n = 1'000'000;

	std::vector<int> nums {};
	std::random_device rd {};
	std::mt19937 mt { rd() };
	std::uniform_int_distribution<int> dist { 10, n * 10 };
	std::unordered_set<int> set {};

	for (int i = 0; i < n; ++i)
		nums.push_back(dist(mt));

	for (auto x : nums) {
		set.insert(x);
		t.put({ x, x });
	}

	printf("num count %zu tree height %zu size %zu expect size %zu\n",
	       nums.size(),
	       t.height(),
	       t.size(),
	       set.size());
	for (auto x : nums) {
		auto v = t.get(x);
		if (x != v->key) {
			printf("bad case1\n");
			std::terminate();
		}
	}

	printf("get test done\n");
	std::uniform_int_distribution<int> d { 0, (n / 2) % n };

	for (int i = 0; i < n; ++i) {
		auto x = nums[d(mt)];
		t.del(x);
		set.erase(x);
		if (t.get(x) != nullptr) {
			printf("bad case2\n");
			std::terminate();
		}
	}
	if (t.size() != set.size()) {
		printf("bad case3 expect size %zu actual size %zu",
		       set.size(),
		       t.size());
		std::terminate();
	}

	t.clear();

	for (auto x : nums)
		t.put({ x, x });

	for (auto x : nums) {
		t.del(x);
		if (t.get(x) != nullptr) {
			printf("bad case4");
			std::terminate();
		}
	}
}

void range_test()
{
	nm::BpTree<Policy> t {};

	for (int i = 1; i < 5; ++i)
		t.put({ i, i });
	t.put({ -1, -1 });

	auto boundary_test = [](auto &&it, std::vector<int> &&expect)
	{
		std::vector<int> actual {};
		while (it) {
			actual.push_back(it.data().key);
			++it;
		}
		assert(expect == actual);
	};

	boundary_test(t.range(-1, 0), { -1 });

	boundary_test(t.range(-1, 1), { -1, 1 });

	boundary_test(t.range(-1, -1), { -1 });

	boundary_test(t.range(4, 5), { 4 });

	boundary_test(t.range(4, 4), { 4 });

	boundary_test(t.range(1, 4), { 1, 2, 3, 4 });

	// seek test
	auto it = t.range(-1, 5);
	std::vector<int> expect = { -1, 1, 2, 3, 4 };
	std::vector<int> r { expect.rbegin(), expect.rend() };
	std::vector<int> a1 {}, a2 {};

	it.seek_end();
	while (it) {
		a1.push_back(it.data().key);
		--it;
	}
	assert(a1 == r);

	it.seek_beg();
	while (it) {
		a2.push_back(it.data().key);
		++it;
	}
	assert(a2 == expect);
}

int main()
{
	base_test();
	range_test();
}
