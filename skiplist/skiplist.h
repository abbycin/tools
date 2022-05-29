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
#include <stdexcept>
#include <utility>

namespace nm
{
template<typename K, typename V, int LEVEL = 8>
class SkipList {
	static_assert(LEVEL > 0, "level must greater than 0");
	class Node {
	public:
		Node(int level) : key_ {}, val_ {}, level_ { level }
		{
			layer_ = new Node *[level_ + 1] { 0 };
		}

		Node(K k, V v, int level)
			: key_ { k }, val_ { v }, level_ { level }
		{
			// level + 1, since we will index using level, namely
			// range in [0, level]
			layer_ = new Node *[level + 1] { 0 };
		}

		~Node()
		{
			delete[] layer_;
		}

	private:
		friend SkipList<K, V, LEVEL>;
		K key_;
		V val_;
		int level_;
		Node **layer_;
	};

public:
	class iterator {
	public:
		iterator(Node *n) : node_ { n }
		{
			if (node_)
				p_ = std::make_pair(node_->key_, node_->val_);
		}

		iterator &operator++()
		{
			if (node_)
				new (this)iterator{node_->layer_[0]};
			else
				throw std::out_of_range("iterator out range");
			return *this;
		}

		std::pair<K, V>& operator *()
		{
			return p_;
		}

		K &key()
		{
			return node_->key_;
		}

		V &val()
		{
			return node_->val_;
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
		std::pair<K, V> p_;
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
		int level;
		int i;

		for (i = cur_; i >= 0; i--) {
			while (cur->layer_[i] && cur->layer_[i]->key_ < key)
				cur = cur->layer_[i];

			update[i] = cur;
		}

		cur = cur->layer_[0];

		// already exists
		if (cur && cur->key_ == key)
			return false;

		level = gen_random_level();

		// in this case, we create a new level by set update to header_
		if (level > cur_) {
			for (i = cur_ + 1; i < level + 1; i++)
				update[i] = header_;
			cur_ = level;
		}

		cur = new Node { key, value, level };

		// insert to each level
		for (i = 0; i <= level; i++) {
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
			while (cur->layer_[i] && cur->layer_[i]->key_ < key)
				cur = cur->layer_[i];
		}

		return cur && cur->layer_[0]->key_ == key;
	}

	std::pair<iterator, iterator> range(K from, K to)
	{
		if (from > to)
			throw std::out_of_range("from is large than to");

		Node *f = search(from);
		Node *t = search(to);

		if (t && t->layer_[0])
			t = t->layer_[0];
		return std::make_pair(iterator { f }, iterator { t });
	}

	iterator begin()
	{
		return { header_->layer_[0] };
	}

	iterator end()
	{
		return { nullptr };
	}

	bool remove(K key)
	{
		Node *cur = header_;
		Node *update[LEVEL + 1] { 0 };
		int i;

		for (i = cur_; i >= 0; i--) {
			while (cur->layer_[i] && cur->layer_[i]->key_ < key)
				cur = cur->layer_[i];
			update[i] = cur;
		}

		cur = cur->layer_[0];
		if (cur && cur->key_ == key) {
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

	size_t size()
	{
		return size_;
	}

private:
	int cur_;
	size_t size_;
	Node *header_;

	static int gen_random_level()
	{
		int k = 1;

		while (rand() % 2) {
			k++;
		}
		k = (k < LEVEL) ? k : LEVEL;
		return k;
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

	Node* search(K key)
	{
		Node *cur = header_;

		for (int i = cur_; i >= 0; --i) {
			while (cur->layer_[i] && cur->layer_[i]->key_ < key)
				cur = cur->layer_[i];
		}
		if (cur)
			cur = cur->layer_[0];
		return cur;
	}
};
}

#endif // SKIPLIST_1653396693_H_
