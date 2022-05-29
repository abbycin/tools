// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2022-05-24 20:51:33
 */

#ifndef SKIPLIST_1653396693_H_
#define SKIPLIST_1653396693_H_

#include <cstddef>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <utility>

namespace nm
{
template<typename K, typename V, int LEVEL = 4>
class SkipList {
	static_assert(LEVEL > 0, "level must greater than 0");
	class Node {
	public:
		Node(int level) : data_ {}, level_ { level }
		{
			layer_ = new Node *[level_ + 1] { 0 };
		}

		Node(K k, V v, int level) : data_ { k, v }, level_ { level }
		{
			// level + 1, since we will index using level, namely
			// range in [0, level]
			layer_ = new Node *[level + 1] { 0 };
		}

		~Node()
		{
			delete[] layer_;
		}

		const K &key() const
		{
			return data_.first;
		}

	private:
		friend SkipList<K, V, LEVEL>;
		std::pair<K, V> data_;
		int level_;
		Node **layer_;
	};

public:
	class iterator {
	public:
		iterator(Node *n, int level) : level_ { level }, node_ { n }
		{
		}

		// no check
		iterator &operator++()
		{
			node_ = node_->layer_[level_];
			return *this;
		}

		const std::pair<K, V> &operator*() const
		{
			return node_->data_;
		}

		const std::pair<K, V> *operator->() const
		{
			return &node_->data_;
		}

		operator bool()
		{
			return node_ != nullptr;
		}

		friend bool operator==(const iterator &l, const iterator &r)
		{
			return l.node_ == r.node_;
		}

		friend bool operator!=(const iterator &l, const iterator &r)
		{
			return !(l == r);
		}

	private:
		int level_;
		Node *node_;
	};

	SkipList() : cur_ { 0 }, size_ { 0 }
	{
		header_ = new Node { LEVEL };
	}

	~SkipList()
	{
		cleanup(header_);
		header_ = nullptr;
		size_ = 0;
		cur_ = 0;
	}

	bool insert(K key, V value)
	{
		Node *update[LEVEL + 1] { 0 };
		Node *cur = header_;
		int rand_level;
		int i;

		for (i = cur_; i >= 0; i--) {
			while (cur->layer_[i] && cur->layer_[i]->key() < key)
				cur = cur->layer_[i];

			update[i] = cur;
		}

		cur = cur->layer_[0];

		// already exists
		if (cur && cur->key() == key)
			return false;

		rand_level = level(d_, g_);

		// in this case, we create a new level by set update to header_
		if (rand_level > cur_) {
			for (i = cur_ + 1; i < rand_level + 1; i++)
				update[i] = header_;
			cur_ = rand_level;
		}

		cur = new Node { key, value, rand_level };

		// insert to each level
		for (i = 0; i <= rand_level; i++) {
			cur->layer_[i] = update[i]->layer_[i];
			update[i]->layer_[i] = cur;
		}

		size_++;
		return true;
	}

	bool contains(K key)
	{
		Node *cur = header_;

		for (int i = cur_; i >= 0; i--) {
			while (cur->layer_[i] && cur->layer_[i]->key() < key)
				cur = cur->layer_[i];
		}

		return cur && cur->layer_[0] && cur->layer_[0]->key() == key;
	}

	// range in [from, to)
	std::pair<iterator, iterator> range(K from, K to)
	{
		if (from > to)
			throw std::out_of_range("from is large than to");

		Node *f = search(from);
		Node *t = search(to);

		return std::make_pair(iterator { f, 0 }, iterator { t, 0 });
	}

	iterator begin()
	{
		return { header_->layer_[0], 0 };
	}

	iterator end()
	{
		return { nullptr, 0 };
	}

	iterator begin(int level)
	{
		return { header_->layer_[level], level };
	}

	bool remove(K key)
	{
		Node *cur = header_;
		Node *update[LEVEL + 1] { 0 };
		int i;

		for (i = cur_; i >= 0; i--) {
			while (cur->layer_[i] && cur->layer_[i]->key() < key)
				cur = cur->layer_[i];
			update[i] = cur;
		}

		cur = cur->layer_[0];
		if (cur && cur->key() == key) {
			// remove from each level
			for (int i = 0; i <= cur_; i++) {
				// this level has no cur
				if (update[i]->layer_[i] != cur)
					break;
				update[i]->layer_[i] = cur->layer_[i];
			}
			delete cur;
			size_--;
			// reduce cur level if current level has no element
			while (cur_ > 0 && header_->layer_[cur_] == 0)
				cur_--;
			return true;
		}
		return false;
	}

	constexpr int level() const
	{
		return LEVEL;
	}

	size_t size()
	{
		return size_;
	}

private:
	inline static std::random_device rd;
	int cur_;
	size_t size_;
	Node *header_;
	std::mt19937 g_ { rd() };
	std::uniform_int_distribution<> d_ { 0, 1 };

	static int level(std::uniform_int_distribution<> &d, std::mt19937 &g)
	{
		int r;

		for (r = 0; r < LEVEL; ++r)
			if (d(g) == 0)
				break;
		return r;
	}

	// example:
	// Level 0: 0x603000000040 1:2;0x6030000000d0 2:3;0x6030000000a0 3:4;
	// Level 1: 0x603000000040 1:2;0x6030000000d0 2:3;0x6030000000a0 3:4;
	// Level 2: 0x603000000040 1:2;0x6030000000a0 3:4;
	// Level 3: 0x6030000000a0 3:4;
	// Level 4: 0x6030000000a0 3:4;
	// Level 5: 0x6030000000a0 3:4; <--- curr_level_
	// level 0 contains all nodes
	static void cleanup(Node *head)
	{
		Node *next, *node = head->layer_[0];
		while (node) {
			next = node->layer_[0];
			delete node;
			node = next;
		}
		delete head;
	}

	Node *search(K key)
	{
		Node *cur = header_;

		for (int i = cur_; i >= 0; --i) {
			while (cur->layer_[i] && cur->layer_[i]->key() < key)
				cur = cur->layer_[i];
		}
		if (cur)
			cur = cur->layer_[0];
		return cur;
	}
};
}

#endif // SKIPLIST_1653396693_H_
