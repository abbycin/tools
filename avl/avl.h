// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-07-15 18:42:41
 */

#ifndef AVL_TREE_H_
#define AVL_TREE_H_

#include <cstdint>
#include <fstream>
#include <queue>

namespace nm
{
template<typename T>
concept avl_cmp = requires(T l, T r) { l <=> r; };
template<typename Key = int>
	requires avl_cmp<Key>
class AVL {
	struct Node {
		Key key;
		// lower 2 bits indicate diff of height of left - right (ie
		// balance factor)
		// 00 -> -1 01 -> equal 10 -> 1 11 -> unused
		uintptr_t factor;
		Node *left;
		Node *right;

		explicit Node(Key k)
			: key { k }
			, factor { 0 }
			, left { nullptr }
			, right { nullptr }
		{
		}
	};

public:
	AVL() : root_ { nullptr }, size_ { 0 }
	{
	}

	~AVL()
	{
		clear();
	}

	bool emplace(Key key)
	{
		if (insert(&root_, key) == nullptr) {
			size_ += 1;
			return true;
		}
		return false;
	}

	bool remove(Key key)
	{
		auto node = find_node(root_, key);

		if (!node)
			return false;
		remove(&root_, node);
		delete node;
		size_ -= 1;
		return true;
	}

	[[nodiscard]] size_t size() const
	{
		return size_;
	}

	bool find(Key key)
	{
		return find_node(root_, key) != nullptr;
	}

	bool is_avl()
	{
		return is_avl(root_) > -1;
	}

	void draw_tree(const std::string &path)
	{
		std::queue<Node *> q {};
		std::ofstream os { path, std::ios::trunc };

		q.push(root_);
		os << "digraph G{\nnode[shape=\"circle\"]";

		while (!q.empty()) {
			auto n = q.size();
			for (size_t i = 0; i < n; ++i) {
				auto r = q.front();
				q.pop();
				if (r->left) {
					os << r->key << "->" << r->left->key
					   << "\n";
					q.push(r->left);
				}
				if (r->right) {
					os << r->key << "->" << r->right->key
					   << "\n";
					q.push(r->right);
				}
			}
		}
		os << "}\n";
	}

	void clear()
	{
		clear(root_);
		root_ = NULL;
	}

private:
	enum {
		BALANCE_MASK = 3
	};
	Node *root_;
	size_t size_;

	static int is_avl(Node *node)
	{
		if (!node)
			return 0;
		int l = is_avl(node->left);
		int r = is_avl(node->right);
		if (l == -1 || r == -1 || std::abs(l - r) > 1)
			return -1;

		return std::max(l, r) + 1;
	}

	static Node *find_node(Node *node, Key &key)
	{
		while (node) {
			if (node->key > key)
				node = node->left;
			else if (node->key < key)
				node = node->right;
			else
				break;
		}
		return node;
	}

	// NOTE: once a tree has parent, it no longer a 'recursive structure'
	// we can treat `left` as `next` pointer and `right` as `child` pointer
	// as a generic tree presentation
	// struct node {
	//	node *next;
	//	node *child;
	// };
	// post-order
	static void clear(Node *node)
	{
		Node *next;

		while (node) {
			next = node->left;
			// set to nullptr ensure we switch to right child when
			// left subtree was destroyed and back to parent
			node->left = nullptr;
			if (!next) {
				if (node->right) {
					next = node->right;
					// set to nullptr ensure we switch to
					// parent node when right subtree was
					// destroyed
					node->right = nullptr;
				} else {
					next = get_node_parent(node);
					delete node;
				}
			}
			node = next;
		}
	}

	static Node *insert(Node **root, Key &key)
	{
		auto pcur = root;
		Node *cur = nullptr;

		while (*pcur) {
			cur = *pcur;
			if (cur->key > key)
				pcur = &cur->left;
			else if (cur->key < key)
				pcur = &cur->right;
			else
				return cur;
		}
		*pcur = new Node { key };

		// mark current node to balanced `height(r) - height(l) == 0`
		(*pcur)->factor = reinterpret_cast<uintptr_t>(cur) | 1;

		balance(root, *pcur);
		return nullptr;
	}

	static Node *
	swap_with_successor(Node **root, Node *X, bool &left_removed)
	{
		Node *Y, *ret, *Q;

		Y = X->right;
		if (!Y->left) {
			ret = Y;
			left_removed = false;
		} else {
			do {
				Q = Y;
				Y = Y->left;
			}
			while (Y->left);

			Q->left = Y->right;
			if (Q->left)
				set_node_parent(Q->left, Q);
			Y->right = X->right;
			set_node_parent(X->right, Y);
			ret = Q;
			left_removed = true;
		}
		Y->left = X->left;
		set_node_parent(X->left, Y);

		Y->factor = X->factor;
		replace_node_child(root, get_node_parent(X), X, Y);
		return ret;
	}

	static Node *
	fixup_remove(Node **root, Node *parent, int sign, bool &left_removed)
	{
		Node *node;
		int ob, nb;

		ob = get_balance_mask(parent);

		if (ob == 0) {
			fix_balance(parent, sign);
			return nullptr;
		}

		nb = ob + sign;
		if (nb == 0) {
			fix_balance(parent, sign);
			node = parent;
		} else {
			node = get_node_child(parent, sign);
			if (sign * get_balance_mask(node) >= 0) {
				single_rotate(root, node, parent, -sign);
				if (get_balance_mask(node) == 0) {
					fix_balance(node, -sign);
					return nullptr;
				} else {
					fix_balance(parent, -sign);
					fix_balance(node, -sign);
				}
			} else {
				node = double_rotate(root, node, parent, -sign);
			}
		}
		parent = get_node_parent(node);
		if (parent)
			left_removed = (node == parent->left);
		return parent;
	}

	static void remove(Node **root, Node *node)
	{
		Node *parent = nullptr, *child;
		bool left_removed = false;

		if (node->left && node->right) {
			parent = swap_with_successor(root, node, left_removed);
		} else {
			child = node->left ? node->left : node->right;
			parent = get_node_parent(node);

			if (child)
				set_node_parent(child,
						parent); // parent maybe NULL
			if (parent) {
				if (node == parent->left) {
					parent->left = child;
					left_removed = true;
				} else {
					parent->right = child;
					left_removed = false;
				}
			} else {
				*root = child;
				return;
			}
		}

		do {
			if (left_removed)
				parent = fixup_remove(
					root, parent, 1, left_removed);
			else
				parent = fixup_remove(
					root, parent, -1, left_removed);
		}
		while (parent);
	}

	static constexpr Node *get_node_parent(Node *node)
	{
		return reinterpret_cast<Node *>(node->factor & ~BALANCE_MASK);
	}

	static constexpr Node *get_node_child(Node *node, int sign)
	{
		return sign < 0 ? node->left : node->right;
	}

	static constexpr void set_node_child(Node *parent, Node *node, int sign)
	{
		if (sign < 0)
			parent->left = node;
		else
			parent->right = node;
	}

	static constexpr void set_node_parent(Node *node, Node *parent)
	{
		node->factor = reinterpret_cast<uintptr_t>(parent) |
			(node->factor & BALANCE_MASK);
	}

	static constexpr void
	replace_node_child(Node **root, Node *parent, Node *oc, Node *nc)
	{
		if (parent) {
			if (oc == parent->left)
				parent->left = nc;
			else
				parent->right = nc;
		} else {
			*root = nc;
		}
	}

	static constexpr void fix_balance(Node *node, int fix)
	{
		node->factor += fix;
	}

	static constexpr void
	fix_parent_balance(Node *node, Node *parent, int sign)
	{
		node->factor = reinterpret_cast<uintptr_t>(parent) | (sign + 1);
	}

	static constexpr int get_balance_mask(Node *node)
	{
		// NOTE: -1 is necessary here which map the factor to -1 0 1
		// since
		// 0b00 == 0 => -1
		// 0b01 == 1 => 0
		// 0b10 == 2 => 1
		return static_cast<int>(node->factor & BALANCE_MASK) - 1;
	}

	static void balance(Node **root, Node *item)
	{
		Node *node = item, *parent;
		bool ok = false;

		parent = get_node_parent(node);
		if (!parent)
			return;

		if (node == parent->left)
			fix_balance(parent, -1);
		else
			fix_balance(parent, 1);

		if (get_balance_mask(parent) == 0)
			return;

		// fixup bottom up
		do {
			node = parent;
			parent = get_node_parent(node);
			if (!parent)
				return;

			if (node == parent->left)
				ok = fixup_insert(root, node, parent, -1);
			else
				ok = fixup_insert(root, node, parent, 1);
		}
		while (!ok);
	}

	static bool
	fixup_insert(Node **root, Node *node, Node *parent, int sign)
	{
		auto old_balance = get_balance_mask(parent);
		auto new_balance = old_balance + sign;

		// current node is balanced, after fix balance it also an avl
		if (old_balance == 0) {
			fix_balance(parent, sign);
			return false;
		}

		// unbalance before but now balanced by adding sign, stop
		// upwards
		if (new_balance == 0) {
			fix_balance(parent, sign);
			return true;
		}

		// node was inserted to left child of left subtree or
		// right child of right subtree, preform single rotate
		if (sign * get_balance_mask(node) > 0) {
			// NOTE: > 0 indicate sign and balance have same `sign`
			// it means both left or both right, in each case we
			// need reduce balance mask so we have to `-sign`
			single_rotate(root, node, parent, -sign);
			fix_balance(parent, -sign);
			fix_balance(node, -sign);
		} else {
			// NOTE: `-sign` means rotate to different direction
			// against current direction
			double_rotate(root, node, parent, -sign);
		}
		return true;
	}

	/*
	 * Template for performing a single rotation ---
	 *
	 * sign > 0:  Rotate clockwise (right) rooted at A:
	 *
	 *           P?            P?
	 *           |             |
	 *           A             B
	 *          / \           / \
	 *         B   C?  =>    D?  A
	 *        / \               / \
	 *       D?  E?            E?  C?
	 *
	 * (nodes marked with ? may not exist)
	 *
	 * sign < 0:  Rotate counterclockwise (left) rooted at A:
	 *
	 *           P?            P?
	 *           |             |
	 *           A             B
	 *          / \           / \
	 *         C?  B   =>    A   D?
	 *            / \       / \
	 *           E?  D?    C?  E?
	 *
	 * This updates pointers but not balance factors!
	 */
	static void single_rotate(Node **root, Node *B, Node *A, int sign)
	{
		auto E = get_node_child(B, sign);
		auto P = get_node_parent(A);

		set_node_child(A, E, -sign);
		set_node_parent(A, B);

		set_node_child(B, A, sign);
		set_node_parent(B, P);

		if (E)
			set_node_parent(E, A);
		replace_node_child(root, P, A, B);
	}

	/*
	 * Template for performing a double rotation ---
	 *
	 * sign > 0:  Rotate counterclockwise (left) rooted at B, then
	 *		     clockwise (right) rooted at A:
	 *
	 *           P?            P?          P?
	 *           |             |           |
	 *           A             A           E
	 *          / \           / \        /   \
	 *         B   C?  =>    E   C? =>  B     A
	 *        / \           / \        / \   / \
	 *       D?  E         B   G?     D?  F?G?  C?
	 *          / \       / \
	 *         F?  G?    D?  F?
	 *
	 * (nodes marked with ? may not exist)
	 *
	 * sign < 0:  Rotate clockwise (right) rooted at B, then
	 *		     counterclockwise (left) rooted at A:
	 *
	 *         P?          P?              P?
	 *         |           |               |
	 *         A           A               E
	 *        / \         / \            /   \
	 *       C?  B   =>  C?  E    =>    A     B
	 *          / \         / \        / \   / \
	 *         E   D?      G?  B      C?  G?F?  D?
	 *        / \             / \
	 *       G?  F?          F?  D?
	 *
	 * Returns a pointer to E and updates balance factors.  Except for those
	 * two things, this function is equivalent to:
	 *	avl_rotate(root_ptr, B, -sign);
	 *	avl_rotate(root_ptr, A, +sign);
	 *
	 * See comment in avl_handle_subtree_growth() for explanation of balance
	 * factor updates.
	 */
	static Node *double_rotate(Node **root, Node *B, Node *A, int sign)
	{
		auto E = get_node_child(B, sign);
		auto F = get_node_child(E, -sign);
		auto G = get_node_child(E, sign);
		auto P = get_node_parent(A);
		auto mask = get_balance_mask(E);

		set_node_child(A, G, -sign);
		fix_parent_balance(A, E, (sign * mask >= 0) ? 0 : -mask);

		set_node_child(B, F, sign);
		fix_parent_balance(B, E, (sign * mask <= 0) ? 0 : -mask);

		set_node_child(E, A, sign);
		set_node_child(E, B, -sign);
		fix_parent_balance(E, P, 0);

		if (G)
			set_node_parent(G, A);
		if (F)
			set_node_parent(F, B);
		replace_node_child(root, P, A, E);
		return E;
	}
};
}

#endif
