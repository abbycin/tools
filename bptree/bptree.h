// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-11-02 14:50:25
 */

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <tuple>

#ifndef BPTREE_1698907825_H_
#define BPTREE_1698907825_H_

namespace nm
{
template<typename Key>
concept BpTreeLess = requires(Key l, Key r) {
	{
		l < r
	} -> std::convertible_to<bool>;
	{
		r < l
	} -> std::convertible_to<bool>;
};

template<typename Policy, int M = 3>
	requires BpTreeLess<typename Policy::key_type>
class BpTree {
public:
	using key_t = typename Policy::key_type;
	using val_t = typename Policy::value_type;

private:
	static_assert(M >= 3, "order must greater than 2");
	enum node_type {
		LEAF_NODE = 1,
		INTL_NODE = 2
	};

	struct node_t;

	struct kc_t {
		key_t key;
		node_t *child;
	};

	struct node_t {
		int type;
		// it's count of keys for leaf node, count of children for
		// internal node
		int count;
		node_t *parent;
		node_t *prev, *next;
	};

	struct leaf_t : node_t {
		val_t data[M];
	};

	// NOTE: key count is count - 1
	struct intl_t : node_t {
		// one extra space for simplify `split` procedure
		kc_t kc[M + 1];
	};

	static node_t *to_node(void *x)
	{
		return static_cast<node_t *>(x);
	}

	static leaf_t *to_leaf(void *x)
	{
		return static_cast<leaf_t *>(x);
	}

	static intl_t *to_intl(void *x)
	{
		return static_cast<intl_t *>(x);
	}

public:
	class iter {
	public:
		iter(leaf_t *beg, leaf_t *end, short b, short e)
			: off_ { b }
			, b_off_ { b }
			, e_off_ { e }
			, cursor_ { beg }
			, head_ { beg }
			, tail_ { end }
		{
			assert(b_off_ <= head_->count);
			assert(e_off_ <= tail_->count);
			if (e_off_ == tail_->count) {
				assert(tail_->count > 0);
				e_off_ -= 1;
			}
		}

		iter()
			: off_ {}
			, b_off_ {}
			, e_off_ {}
			, cursor_ { nullptr }
			, head_ { nullptr }
			, tail_ { nullptr }
		{
		}

		val_t &data()
		{
			return cursor_->data[off_];
		}

		explicit operator bool()
		{
			if (!cursor_)
				return false;

			if (cursor_ == head_ && off_ < b_off_)
				return false;
			if (cursor_ == tail_ && off_ > e_off_)
				return false;
			return true;
		}

		iter &operator++()
		{
			off_ += 1;
			if (off_ >= cursor_->count && cursor_ != tail_) {
				cursor_ = to_leaf(cursor_->next);
				off_ = 0;
			}
			return *this;
		}

		iter &operator--()
		{
			off_ -= 1;
			if (off_ < 0 && cursor_ != head_) {
				cursor_ = to_leaf(cursor_->prev);
				assert(cursor_);
				off_ = cursor_->count - 1;
			}
			return *this;
		}

		void seek_beg()
		{
			cursor_ = head_;
			off_ = b_off_;
		}

		void seek_end()
		{
			cursor_ = tail_;
			off_ = e_off_;
		}

	private:
		int off_;
		short b_off_;
		short e_off_;
		leaf_t *cursor_;
		leaf_t *head_;
		leaf_t *tail_;
	};

	BpTree() : root_ { nullptr }
	{
	}

	~BpTree()
	{
		clear();
	}

	void put(val_t key)
	{
		if (!root_) {
			auto leaf = new leaf_t {};
			leaf->data[0] = std::move(key);
			leaf->count += 1;
			leaf->type = LEAF_NODE;

			root_ = to_node(leaf);
		} else {
			auto l = search(root_, Policy::key(key));
			if (l)
				leaf_put(l, std::move(key));
		}
	}

	val_t *get(key_t key)
	{
		auto l = search(root_, key);
		if (l) {
			auto [ok, pos] = leaf_search(l, key);
			if (ok)
				return &l->data[pos];
		}
		return nullptr;
	}

	void del(key_t key)
	{
		auto l = search(root_, key);
		if (l)
			leaf_del(l, key);
	}

	// return exactly range [from, to] when found, or else return
	// sub-range
	iter range(key_t from, key_t to)
	{
		if (from > to)
			std::swap(from, to);
		auto l = search(root_, from);
		auto r = search(root_, to);
		auto [_b, beg] = leaf_search(l, from);
		auto [_e, end] = leaf_search(r, to);

		if (!_b && !_e && l == r) {
			// both `from` and `to` are not found in same leaft
			if (beg == l->count && end == r->count)
				return {};
		}

		// adjust left boundary
		if (!_b) {
			if (beg == l->count)
				l = to_leaf(l->next);
			if (!l)
				return {};
			beg = 0;
		}

		// adjust right boundary
		if (!_e) {
			if (end == 0)
				r = to_leaf(r->prev);
			if (!r)
				return {};
			end -= r->count - 1;
		}

		return { l, r, (short)beg, (short)end };
	}

	[[nodiscard]] size_t size() const
	{
		size_t n = 0;
		auto cur = root_;
		if (cur) {
			while (cur->type != LEAF_NODE) {
				auto it = to_intl(cur);
				cur = it->kc[0].child;
			}
			while (cur) {
				n += cur->count;
				cur = cur->next;
			}
		}
		return n;
	}

	[[nodiscard]] size_t height() const
	{
		size_t h = 0;
		auto cur = root_;
		if (cur) {
			h += 1;
			while (cur->type != LEAF_NODE) {
				h += 1;
				auto it = to_intl(cur);
				cur = it->kc[0].child;
			}
		}
		return h;
	}

	void clear()
	{
		if (root_) {
			while (root_->type != LEAF_NODE) {
				auto it = to_intl(root_);
				auto next = it->kc[0].child;
				list_clear(root_);
				root_ = next;
			}
			list_clear(root_);
			root_ = nullptr;
		}
	}

private:
	node_t *root_;

	static leaf_t *search(node_t *cur, key_t key)
	{
		while (cur) {
			switch (cur->type) {
			case LEAF_NODE: {
				auto l = to_leaf(cur);
				return l;
			}
			case INTL_NODE: {
				auto n = to_intl(cur);
				auto [ok, pos] = intl_search(n, key);
				if (ok)
					pos += 1;
				cur = n->kc[pos].child;
			}
			}
		}
		return nullptr;
	}

	// there are 4 cases:
	// 1. if leaf is not full, insert and return
	// 2. if leaf is full, split into two, the old holds floor half and the
	// new holds ceil half
	// 3. if in `2.` cause the parent full, split parent recursively
	// 4. if the key is larger than any key in `root_`, insert to last child
	// of `root_` and repeat `2.` and `3.`
	void leaf_put(leaf_t *leaf, val_t &&val)
	{
		auto [ok, pos] = leaf_search(leaf, Policy::key(val));

		// update old value
		if (ok) {
			leaf->data[pos] = val;
			return;
		}

		if (!leaf_is_full(leaf)) {
			// make a space for new key val pair
			rshift(leaf->data, leaf->count, pos);
			leaf->data[pos] = val;
			leaf->count += 1;
			return;
		}

		auto &v = leaf_split(leaf, pos, val);
		insert_fixup(leaf, leaf->next, Policy::key(v));
	}

	// we reserve a space for insert, and then split
	static val_t &leaf_split(leaf_t *leaf, int pos, val_t val)
	{
		auto mid = leaf->count / 2;
		auto node = new leaf_t {};

		node->type = LEAF_NODE;

		list_append(leaf, node);

		rshift(leaf->data, leaf->count, pos);
		leaf->data[pos] = val;
		leaf->count += 1;

		// copy to node
		node->count = leaf->count - mid;
		copy(node->data, leaf->data + mid, node->count);
		leaf->count -= node->count;

		return node->data[0];
	}

	void insert_fixup(void *l, void *r, key_t key)
	{
		auto lhs = to_node(l);
		auto rhs = to_node(r);
		if (!lhs->parent && !rhs->parent) {
			auto parent = new intl_t {};

			parent->type = INTL_NODE;
			parent->count = 2;
			parent->kc[0].key = key;
			parent->kc[0].child = lhs;
			parent->kc[1].child = rhs;

			lhs->parent = to_node(parent);
			rhs->parent = to_node(parent);

			root_ = to_node(parent);
		} else {
			// since we always split the new node in right
			assert(lhs->parent);
			rhs->parent = lhs->parent;
			intl_put(to_intl(rhs->parent), rhs, key);
		}
	}

	void intl_put(intl_t *parent, node_t *child, key_t key)
	{
		auto [ok, pos] = intl_search(parent, key);
		assert(!ok);

		if (!intl_is_full(parent)) {
			// NOTE: the old child remain unchanged, since rshift is
			// copying not moving, so we only need to set pos + 1 to
			// the new child
			rshift(parent->kc, parent->count, pos);
			parent->kc[pos].key = key;
			parent->kc[pos + 1].child = child;
			parent->count += 1;
			return;
		}

		key = intl_split(parent, child, pos, key);
		auto right_sibling = to_intl(parent->next);

		insert_fixup(parent, right_sibling, key);
	}

	static key_t intl_split(intl_t *node, node_t *child, int pos, key_t key)
	{
		// the ceil half
		int mid = (node->count + 1) / 2;
		auto rhs = new intl_t {};

		rhs->type = INTL_NODE;
		list_append(node, rhs);

		// for example:
		//   | 1 | 2  | 3  |    |
		//   | c | c1 | p2 | c3 |
		// we need copy the last two keys and children, the middle key 2
		// is going to move to parent
		// NOTE: we reserved a space for insert, then split
		rshift(node->kc, node->count, pos);
		node->kc[pos].key = key;
		node->kc[pos + 1].child = child;
		node->count += 1;
		// the old child at `pos` is not overwritten, do nothing
		// the key transfer to parent
		key_t rkey = node->kc[mid - 1].key;

		// split node
		rhs->count = node->count - mid;
		for (int i = mid, j = 0; j < rhs->count; ++i, ++j) {
			rhs->kc[j] = node->kc[i];
			if (rhs->kc[j].child)
				rhs->kc[j].child->parent = to_node(rhs);
		}
		node->count -= rhs->count;
		return rkey;
	}

	static void list_append(node_t *node, node_t *x)
	{
		x->next = node->next;
		if (node->next)
			node->next->prev = x;
		x->prev = node;
		node->next = x;
	}

	static void list_del(node_t *node)
	{
		auto prev = node->prev;
		auto next = node->next;

		if (prev)
			prev->next = next;
		if (next)
			next->prev = prev;

		if (node->type == LEAF_NODE)
			delete to_leaf(node);
		else
			delete to_intl(node);
	}

	void list_clear(node_t *head)
	{
		node_t *next;
		while (head) {
			next = head->next;
			if (head->type == LEAF_NODE)
				delete to_leaf(head);
			else
				delete to_intl(head);
			head = next;
		}
	}

	static bool leaf_overhalf(leaf_t *leaf)
	{
		return leaf->count > (M + 1) / 2;
	}

	static bool intl_overhalf(intl_t *it)
	{
		return it->count > (M + 1) / 2;
	}

	// NOTE: the `M - 1` slot is reserved for new key-value pairs before
	// performing split, this simplifies the split procedure
	static bool leaf_is_full(leaf_t *leaf)
	{
		return leaf->count == M - 1;
	}

	// NOTE: when the child count == M then key count is M - 1
	static bool intl_is_full(intl_t *it)
	{
		return it->count == M;
	}

	template<typename T>
	static void rshift(T *arr, int size, int pos)
	{
		size -= pos;
		if (size > 0)
			memmove(arr + pos + 1, arr + pos, size * sizeof(T));
	}

	template<typename T>
	static void lshift(T *arr, int size, int pos)
	{
		size -= (pos + 1);
		if (size > 0)
			memmove(arr + pos, arr + pos + 1, size * sizeof(T));
	}

	template<typename T>
	static void copy(T *dst, T *src, int count)
	{
		memcpy(dst, src, count * sizeof(T));
	}

	static const key_t &key_of(val_t &v)
	{
		return Policy::key(v);
	}

	static const key_t &key_of(kc_t &v)
	{
		return v.key;
	}

	template<typename T>
	static int bsearch(T arr[], int n, key_t key)
	{
		int l = 0;
		int r = n - 1;

		while (l <= r) {
			int m = l + (r - l) / 2;
			if (key_of(arr[m]) >= key)
				r = m - 1;
			else
				l = m + 1;
		}
		return l;
	}

	static std::tuple<bool, int> leaf_search(leaf_t *leaf, key_t key)
	{
		auto pos = bsearch(leaf->data, leaf->count, key);

		if (pos < leaf->count && Policy::key(leaf->data[pos]) == key)
			return { true, pos };
		return { false, pos };
	}

	// find child index, when found, the index is `pos` or else it should be
	// `pos+1`, for example:
	// the root is [9, 11] and it has 3 leaves [1, 4] [9, 10] and [11, 12]
	// when `key = 5` the pos will be `0` which less than key_count, and we
	// know the 5 may exist in `kc[pos].child`
	// when the `key = 10` the pos will be `1` which equal to key_count, and
	// we know the 10 may exist in `kc[pos+1].child`
	static std::tuple<bool, int> intl_search(intl_t *it, key_t key)
	{
		// NOTE: we are search by keys, and the keys' count is one less
		// than child count
		assert(it->count > 0);
		auto key_count = it->count - 1;
		auto pos = bsearch(it->kc, key_count, key);

		if (pos < key_count && it->kc[pos].key == key)
			return { true, pos };
		// NOTE: here the pos may less than or equal to key_count, which
		// is the insert pos of the given key
		return { false, pos };
	}

	// find the key's index in parent, since `intl_search` return the insert
	// position of child and key index equal to child index - 1, so when not
	// found, the key's index should subtract 1
	//
	// the index is used for rotation when perform borrow or merge operation
	// for example:
	// borrow a kv from right sibling, before that the node itself, parent
	// and right sibling have order node < parent < right, to not violate
	// that order,  we need move key of parent down to node's last slot and
	// then move right first key to parent, and finally shift all slots in
	// right to left
	//
	// it's more efficient to track the index in `node_t`, but on the other
	// hand, the code will be more complex and error-prone
	static int key_index_in_parent(intl_t *parent, key_t key)
	{
		auto [ok, pos] = intl_search(parent, key);
		if (!ok)
			pos -= 1;
		return pos;
	}

	// idx is key's index in parent, since the child's insert position is in
	// range [0, p->count-1], then key's index is in range [-1, p->count-2]
	// return 1 for operate on right, 0 for the left, we prefer operate on
	// left side
	static int which_side(intl_t *p, int idx, node_t *l, node_t *r)
	{
		if (idx == -1)
			return 1;
		if (idx == p->count - 2)
			return 0;
		return l->count >= r->count ? 0 : 1;
	}

	// there are 3 cases:
	// 1. leaf count overhalf, simple remove elem and return
	// 2. if in step 1. the key to be deleted is the first one, then the
	// parent's key should be updated
	// 3. leaf count too low, borrow one from left or right if possible:
	//    - if leaf is first node, merge right to leaf, delete right
	//    - if leaf is last node, merge leaf to left, delete leaf
	//    - if both left and right exist, borrow from the larger count one
	// 4. no borrow can be made, merge sibling nodes
	void leaf_del(leaf_t *leaf, key_t key)
	{
		auto [ok, pos] = leaf_search(leaf, key);
		if (!ok)
			return;

		if (leaf_overhalf(leaf))
			return leaf_simple_del(leaf, pos);

		auto parent = to_intl(leaf->parent);
		if (!parent) {
			if (leaf->count == 1) {
				list_del(leaf);
				root_ = nullptr;
			} else {
				leaf_simple_del(leaf, pos);
			}
			return;
		}

		auto idx =
			key_index_in_parent(parent, Policy::key(leaf->data[0]));
		int right = which_side(parent, idx, leaf->prev, leaf->next);
		auto l = to_leaf(leaf->prev);
		auto r = to_leaf(leaf->next);

		leaf_simple_del(leaf, pos);

		// NOTE: the operation direction is always from right to left
		// borrow or merge right operate on right's parent, so idx + 1
		// while borrow or merge to left operate on node's parent, idx
		// remain unchanged
		if (right) {
			idx += 1;
			if (leaf_overhalf(r)) {
				leaf_borrow_rhs(parent, leaf, r, idx);
			} else {
				leaf_merge_rhs(leaf, r);
				intl_del(parent, idx);
			}
		} else {
			assert(idx >= 0);
			if (leaf_overhalf(l)) {
				leaf_borrow_lhs(parent, leaf, l, idx);
			} else {
				leaf_merge_lhs(leaf, l);
				intl_del(parent, idx);
			}
		}
	}

	static void
	leaf_borrow_rhs(intl_t *parent, leaf_t *leaf, leaf_t *r, int idx)
	{
		// borrow one to the end of leaf
		leaf->data[leaf->count] = r->data[0];
		leaf->count += 1;
		// remove the borrowed one
		leaf_simple_del(r, 0);
		parent->kc[idx].key = Policy::key(r->data[0]);
	}

	static void
	leaf_borrow_lhs(intl_t *parent, leaf_t *leaf, leaf_t *l, int idx)
	{
		rshift(leaf->data, leaf->count, 0);
		leaf->data[0] = l->data[l->count - 1];
		leaf->count += 1;
		l->count -= 1;
		parent->kc[idx].key = Policy::key(leaf->data[0]);
	}

	static void leaf_merge_rhs(leaf_t *leaf, leaf_t *r)
	{
		copy(leaf->data + leaf->count, r->data, r->count);
		leaf->count += r->count;
		list_del(r);
	}

	static void leaf_merge_lhs(leaf_t *leaf, leaf_t *l)
	{
		copy(l->data + l->count, leaf->data, leaf->count);
		l->count += leaf->count;
		list_del(leaf);
	}

	static void leaf_simple_del(leaf_t *leaf, int pos)
	{
		lshift(leaf->data, leaf->count, pos);
		leaf->count -= 1;
	}

	void intl_del(intl_t *node, int pos)
	{
		if (intl_overhalf(node))
			return intl_simple_del(node, pos);

		auto parent = to_intl(node->parent);

		if (!parent) {
			// the last one, reduce tree height
			if (node->count == 2) {
				node->kc[0].child->parent = nullptr;
				// it's why we prefer merge into left
				root_ = node->kc[0].child;
				list_del(node);
			} else {
				intl_simple_del(node, pos);
			}
			return;
		}

		auto idx = key_index_in_parent(parent, node->kc[0].key);
		int right = which_side(parent, idx, node->prev, node->next);
		auto l = to_intl(node->prev);
		auto r = to_intl(node->next);

		if (right) {
			idx += 1;
			intl_simple_del(node, pos);
			if (intl_overhalf(r)) {
				intl_borrow_rhs(parent, node, r, idx);
			} else {
				intl_merge_rhs(parent, node, r, idx);
				intl_del(parent, idx);
			}
		} else {
			assert(idx >= 0);
			// skipp the key and child at pos while shifting instead
			// of delete, it's more simple and efficient than invoke
			// intl_simple_del and then move all data, since the key
			// and child are not correspond one-to-one
			if (intl_overhalf(l)) {
				intl_borrow_lhs(parent, node, l, pos, idx);
			} else {
				intl_merge_lhs(parent, node, l, pos, idx);
				intl_del(parent, idx);
			}
		}
	}

	static void intl_borrow_rhs(intl_t *p, intl_t *node, intl_t *r, int idx)
	{
		// left rotation, put the parent key to the left, and then put
		// the right key to the parent, and finally left shift the right
		// to keep the order lhs < parent < rhs
		node->kc[node->count - 1].key = p->kc[idx].key;
		// update parent key to larger one
		p->kc[idx].key = r->kc[0].key;

		// borrow first child from right
		node->kc[node->count].child = r->kc[0].child;
		node->kc[node->count].child->parent = to_node(node);
		node->count += 1;

		// remove borrowed kc from right
		for (int i = 0; i < r->count - 2; ++i)
			r->kc[i].key = r->kc[i + 1].key;
		for (int i = 0; i < r->count - 1; ++i)
			r->kc[i].child = r->kc[i + 1].child;

		r->count -= 1;
	}

	static void
	intl_borrow_lhs(intl_t *p, intl_t *node, intl_t *l, int pos, int idx)
	{
		// reserve one slot at 0 for borrowing key and child
		for (int i = pos; i > 0; --i)
			node->kc[i].key = node->kc[i - 1].key;
		for (int i = pos + 1; i > 0; --i)
			node->kc[i].child = node->kc[i - 1].child;

		// right rotation, put the parent key to the right, and then put
		// the left key to the parent, and finally remove the last key
		// from left (simply reduce its size) to keep the order
		// left last < parent at `idx` < node first
		node->kc[0].key = p->kc[idx].key;
		p->kc[idx].key = l->kc[l->count - 2].key;

		node->kc[0].child = l->kc[l->count - 1].child;
		node->kc[0].child->parent = to_node(node);
		l->count -= 1;
	}

	static void intl_merge_rhs(intl_t *p, intl_t *node, intl_t *r, int idx)
	{
		// the key is corresponding to the child of `r` in first slot
		node->kc[node->count - 1].key = p->kc[idx].key;

		for (int i = node->count, j = 0; j < r->count - 1; ++i, ++j)
			node->kc[i].key = r->kc[j].key;

		for (int i = node->count, j = 0; j < r->count; ++i, ++j) {
			node->kc[i].child = r->kc[j].child;
			if (node->kc[i].child)
				node->kc[i].child->parent = to_node(node);
		}
		node->count += r->count;
		list_del(r);
	}

	static void
	intl_merge_lhs(intl_t *p, intl_t *node, intl_t *l, int pos, int idx)
	{
		// the key is corresponding to the child of `node` in first slot
		l->kc[l->count - 1].key = p->kc[idx].key;

		for (int i = l->count, j = 0; j < node->count - 1; ++j) {
			if (j != pos) {
				l->kc[i].key = node->kc[j].key;
				i += 1;
			}
		}

		for (int i = l->count, j = 0; j < node->count; ++j) {
			if (j == pos + 1)
				continue;
			l->kc[i].child = node->kc[j].child;
			if (l->kc[i].child)
				l->kc[i].child->parent = to_node(l);
			i += 1;
		}

		l->count += node->count - 1;
		list_del(node);
	}

	// NOTE: pos is the key's index, child index should be pos + 1
	static void intl_simple_del(intl_t *node, int pos)
	{
		assert(node->count >= 2);
		for (int i = pos; i < node->count - 2; ++i) {
			node->kc[i].key = node->kc[i + 1].key;
			// we have one extra space, so i + 2 is valid
			node->kc[i + 1].child = node->kc[i + 2].child;
		}
		node->count -= 1;
	}
};

}

#endif // BPTREE_1698907825_H_
