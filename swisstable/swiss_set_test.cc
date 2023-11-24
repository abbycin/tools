// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-09-22 19:37:11
 */

#include "swiss_set.h"
#include <instant/instant.h>
#include <random>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>

struct Hash {
	static uint64_t hash(const int &x)
	{
		return nm::HashFn::murmur(&x, sizeof(x));
	}

	static uint64_t hash(const std::string &x)
	{
		return nm::HashFn::murmur(x.data(), x.size());
	}

	uint64_t operator()(const int &x) const
	{
		return nm::HashFn::murmur(&x, sizeof(x));
	}

	uint64_t operator()(const std::string &x) const
	{
		return nm::HashFn::murmur(x.data(), x.size());
	}
};

template<typename T, typename Set, typename H = Hash>
void bench(std::vector<T> &v, Set &s, bool ins)
{
	auto b = nm::Instant::now();
	volatile int64_t junk = 0;
	if (ins)
		for (auto &x : v)
			s.insert(x);
	else
		for (auto &x : v)
			junk += s.contains(x);
	auto ms = b.elapse_ms();
	if constexpr (std::is_same_v<nm::SwissSet<T, H>, Set>)
		printf("%20s => %6fms\n", "swiss", ms);
	else
		printf("%20s => %6fms\n", "std::unordered_set", ms);
}

template<typename T, typename H = Hash>
void bench()
{
	int n = 1000000;
	std::random_device rd {};
	std::mt19937 gen { rd() };
	std::uniform_int_distribution<int> dis { -n, n };
	std::vector<T> v {};
	v.reserve(n);
	for (int i = 0; i < n; ++i) {
		if constexpr (std::is_same_v<T, std::string>)
			v.push_back(std::to_string(dis(gen)));
		else
			v.push_back(dis(gen));
	}

	std::unordered_set<T, H> s {};
	nm::SwissSet<T, H> sw {};

	s.reserve(n);
	sw.reserve(n);
	std::string name = "int";

	if constexpr (std::is_same_v<std::string, T>)
		name = "string";

	printf("----------- %8s insert ------------\n", name.c_str());
	bench(v, s, true);
	bench(v, sw, true);

	printf("----------- %8s search ------------\n", name.c_str());
	bench(v, s, false);
	bench(v, sw, false);
	printf("unordered_set cap %lu size %lu load_factor %f\n",
	       s.bucket_count(),
	       s.size(),
	       s.load_factor());
	printf("swisstable    cap %lu size %lu load_factor %f\n",
	       sw.cap(),
	       sw.size(),
	       sw.load_factor());
}

template<typename K, typename V>
class Item {
public:
	Item(const K &k, const V &v) : key { k }, val { v }
	{
	}

	Item(const Item &rhs) : key { rhs.key }, val { rhs.val }
	{
	}

	Item(Item &&rhs) noexcept
		: key { std::move(rhs.key) }, val { std::move(rhs.val) }
	{
	}
	~Item() = default;

	const void *data() const
	{
		if constexpr (std::is_trivial_v<K>)
			return &key;
		else
			return key.data();
	}

	uint64_t size() const
	{
		if constexpr (std::is_trivial_v<K>)
			return sizeof(K);
		else
			return key.size();
	}

	friend bool operator==(const Item &l, const Item &r)
	{
		return l.key == r.key;
	}

	friend bool operator==(const Item &l, const K &r)
	{
		return l.key == r;
	}

	friend std::ostream &operator<<(std::ostream &os, const Item &i)
	{
		os << i.key << " => " << i.val;
		return os;
	}

	K key;
	V val;
};

void heterogeneous()
{
	printf("------------ heterogeneous find test ------------\n");
	using namespace std::string_literals;
	using namespace std::string_view_literals;
	{
		nm::SwissSet<Item<int, std::string>> s {};

		s.insert({ 1, "2" });
		s.insert({ 2, "3" });
		s.emplace(3, "4");

		auto it = s.find(Item { 1, ""s });
		std::cout << "find by item " << *it << '\n';

		it = s.find(2);
		std::cout << "find by key  " << *it << '\n';

		printf("============ key => val =================\n");
		for (auto &[k, v] : s)
			std::cout << k << " => " << v << '\n';
	}

	{
		using X = Item<std::string, int>;

		// extend for string types
		struct string_hash : nm::SwissHash {
			using nm::SwissHash::hash;

			static uint64_t hash(const std::string &s)
			{
				return nm::HashFn::murmur(s.data(), s.size());
			}
			static uint64_t hash(std::string_view sw)
			{
				return nm::HashFn::murmur(sw.data(), sw.size());
			}
			static uint64_t hash(const char *s)
			{
				return nm::HashFn::murmur(s, strlen(s));
			}
		};

		struct string_eq : nm::SwissEq {
			using nm::SwissEq::eq;

			static bool eq(const X &i, const std::string_view &sw)
			{
				return i.key == sw;
			}
		};
		nm::SwissSet<X, string_hash, string_eq> s { { "3"s, 4 },
							    { "4"s, 5 } };

		s.emplace("1", 2);
		s.emplace("2", 3);

		auto it = s.find(Item { "1"s, 0 });
		std::cout << "find by item " << *it << '\n';

		it = s.find("2"s);
		std::cout << "find by key  " << *it << '\n';

		it = s.find("1"sv);
		std::cout << "find by view " << *it << '\n';

		it = s.find("1");
		std::cout << "find by raw  " << *it << '\n';

		printf("============ key => val =================\n");
		for (auto &[k, v] : s)
			std::cout << k << " => " << v << '\n';
	}
}

int main()
{
	bench<int>();
	bench<std::string>();

	heterogeneous();
}