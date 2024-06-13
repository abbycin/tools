// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2024-06-12 19:42:10
 */

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <optional>
#include <vector>
#include <string>

template<typename T>
class RadixTree {
	struct Node;
	using val_t = std::optional<T>;
	// use pointer to reduce element mvoe cost
	using leaf_t = std::vector<Node *>;

public:
	RadixTree() : root_ {}
	{
	}

	~RadixTree()
	{
		clear();
	}

	template<typename V>
		requires std::convertible_to<V, val_t>
	void add(std::string key, V &&val)
	{
		auto node = &root_;
		for (;;) {
			Node *p = nullptr;
			size_t len = 0;
			for (auto &c : node->leaf) {
				len = common_prefix(c->key, key);
				if (len != 0) {
					p = c;
					break;
				}
			}

			if (!p) {
				node->leaf.push_back(new Node {
					std::move(key), std::move(val) });
				return;
			}

			auto prefix = key.substr(0, len);
			auto rest = p->key.substr(len);

			if (rest.empty()) {
				if (len == key.size()) {
					p->val = val;
					return;
				}
				node = p;
				key = key.substr(len);
			} else {
				auto tmp = new Node { rest,
						      std::move(p->val),
						      std::move(p->leaf) };
				p->key = prefix;
				p->val = std::nullopt;
				p->leaf.push_back(std::move(tmp));
				key = key.substr(len);
				if (key.empty()) {
					p->val = val;
					return;
				}
				node = p;
			}
		}
	}

	T *get(std::string key)
	{
		auto node = &root_;
		for (;;) {
			Node *p = nullptr;
			for (auto &c : node->leaf) {
				int len = common_prefix(key, c->key);
				if (len) {
					key = key.substr(len);
					p = c;
					break;
				}
			}

			if (!p)
				return nullptr;

			if (key.empty())
				return &p->val.value();
			node = p;
		}
	}

	void del(std::string key)
	{
		auto node = &root_;
		bool stop = false;
		while (!stop) {
			stop = true;
			for (size_t i = 0; i < node->leaf.size(); ++i) {
				auto c = node->leaf[i];
				size_t len = common_prefix(key, c->key);
				if (!len)
					continue;

				// full matched
				if (len == key.size())
					return try_remove(node, c, i);
				// continue in sub-tree
				node = c;
				key = key.substr(len);
				stop = false;
				break;
			}
		}
	}

	void clear()
	{
		root_.key.clear();
		root_.val.reset();
		for (auto x : root_.leaf)
			delete x;
		root_.leaf.clear();
	}

private:
	struct Node {
		std::string key {};
		val_t val {};
		leaf_t leaf {};

		Node() = default;

		~Node()
		{
			for (auto x : leaf)
				delete x;
		}

		Node(std::string k, T &&v)
			: key { std::move(k) }, val { std::move(v) }
		{
		}

		template<typename U>
			requires std::convertible_to<T, val_t>
		Node(std::string k, U v, leaf_t l)
			: key { std::move(k) }
			, val { std::move(v) }
			, leaf { std::move(l) }
		{
		}

		Node(Node &&node)
			: key { std::move(node.key) }
			, val { std::move(node.val) }
			, leaf { std::move(node.leaf) }
		{
		}

		void merge()
		{
			this->key += this->leaf[0]->key;
			this->val = std::move(this->leaf[0]->val);
			this->leaf = std::move(this->leaf[0]->leaf);
		}
	};

	Node root_;

	int common_prefix(std::string &lhs, std::string &rhs)
	{
		int n = std::min(lhs.size(), rhs.size());
		int i = 0;

		while (i < n && lhs[i] == rhs[i])
			i += 1;
		return i;
	}

	void try_remove(Node *p, Node *c, int index)
	{
		if (!c->val)
			return;
		c->val = std::nullopt;
		if (c->leaf.empty()) {
			delete p->leaf[index];
			p->leaf.erase(p->leaf.begin() + index);
			if (p->leaf.size() == 1 && !p->val && !p->key.empty())
				p->merge();
		} else if (c->leaf.size() == 1)
			c->merge();
	}
};

#include <fmt/format.h>

int main()
{
	RadixTree<std::string> t {};

	t.add("mo", "ha");
	t.add("m", "h");
	t.add("moh", "moh");

	std::vector<std::string> keys { "mo", "m", "moh" };
	for (auto &k : keys) {
		auto r = t.get(k);
		if (r)
			fmt::println("{} => {}", k, *r);
	}

	t.del("moh");

	fmt::println("==================");

	for (auto &k : keys) {
		auto r = t.get(k);
		if (r)
			fmt::println("{} => {}", k, *r);
	}
}