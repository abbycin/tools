/*********************************************************
	  File Name:hash.h
	  Author: Abby Cin
	  Mail: abbytsing@gmail.com
	  Created Time: Fri 17 Jun 2016 06:45:27 PM CST
**********************************************************/

#ifndef HASH_H_
#define HASH_H_

#include <cassert>
#include <cstring>
#include <string>
#include <cstdint>

namespace nm
{
namespace hash
{
	namespace
	{
		inline uint32_t decode(const char *p)
		{
			uint32_t res;
			std::memcpy(&res, p, sizeof(res));
			return res;
		}
	}

	// MurmurHash2 was written by Austin Appleby, and is placed in the
	// public domain. The author hereby disclaims copyright to this source
	// code. https://github.com/aappleby/smhasher
	uint32_t murmurhash2(const char *data, std::size_t size, uint32_t seed)
	{
		const uint32_t m = 0x5bd1e995;
		const int r = 24;
		uint32_t h = seed ^ size;
		while (size >= 4) {
			uint32_t k = decode(data);
			data += 4;
			k *= m;
			k ^= k >> r;
			k *= m;
			h *= m;
			h ^= k;
			size -= 4;
		}
		switch (size) {
		case 3:
			h ^= static_cast<unsigned char>(data[2]) << 16;
#if __cplusplus >= 201703
			[[fallthrough]];
#endif
		case 2:
			h ^= static_cast<unsigned char>(data[1]) << 8;
#if __cplusplus >= 201703
			[[fallthrough]];
#endif
		case 1:
			h ^= static_cast<unsigned char>(data[0]);
			h *= m;
		}
		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;
		return h;
	}
	class HashTable {
	private:
		struct node {
			std::string key;
			std::string value;
			uint32_t hash;
			int dup_count;
			node *next;
			node *sibling;
			node()
				: key()
				, value()
				, hash()
				, dup_count(0)
				, next(nullptr)
				, sibling(nullptr)
			{
			}
			node(const std::string &k,
			     const std::string &o,
			     uint32_t h)
				: key(k)
				, value(o)
				, hash(h)
				, dup_count(0)
				, next(nullptr)
				, sibling(nullptr)
			{
			}
		};

	public:
		class iterator {
		private:
			node *root;

		public:
			iterator() : root(nullptr)
			{
			}
			iterator(node *n) : root(n)
			{
			}
			iterator(const iterator &rhs)
			{
				*this = rhs;
			}
			~iterator()
			{
			}
			iterator &operator=(const iterator &rhs)
			{
				root = rhs.root;
				return *this;
			}
			iterator &operator++()
			{
				root = root->sibling;
				return *this;
			}
			iterator operator++(int)
			{
				auto tmp = *this;
				++(*this);
				return tmp;
			}
			node &operator*()
			{
				return *root;
			}
			node *operator->()
			{
				return root;
			}
			bool is_valid()
			{
				return root != nullptr;
			}
		};
		HashTable()
			: allow_multi_(false)
			, size_(0)
			, limit_(0)
			, list_(nullptr)
		{
			resize_();
		}
		HashTable(bool allow_multi)
			: allow_multi_(allow_multi)
			, size_(0)
			, limit_(0)
			, list_(nullptr)
		{
			resize_();
		}
		~HashTable()
		{
			for (uint32_t i = 0; i < limit_; ++i) {
				node *ptr = list_[i];
				while (ptr != nullptr) {
					node *tmp = ptr->next;
					if (ptr->sibling) {
						auto root = ptr->sibling;
						while (root != nullptr) {
							auto nxt =
								root->sibling;
							delete root;
							root = nxt;
						}
					}
					delete ptr;
					ptr = tmp;
				}
			}
			delete[] list_;
		}
		uint32_t insert(const std::string &key, const std::string &obj)
		{
			bool is_new_entery = true;
			uint32_t h = murmurhash2(key.data(), key.size(), 0);
			node **ptr = find_(key, h);
			node *old = *ptr;
			node *item = new node(key, obj, h);
			if (old != nullptr) // key already in table, replace
					    // with new item
			{
				if (allow_multi_) {
					item->sibling = old->sibling;
					old->sibling = item;
					old->dup_count += 1;
					return h; // insert finished
				} else {
					is_new_entery = false;
					node *tmp = old;
					old = tmp->next;
					delete tmp;
					tmp = nullptr;
				}
			}
			item->next = (old == nullptr ? nullptr : old->next);
			*ptr = item;
			if (old == nullptr) {
				if (is_new_entery)
					++size_;
				if (size_ > limit_)
					resize_();
			}
			return h;
		}
		void remove(const std::string &key, uint32_t hash)
		{
			node **ptr = find_(key, hash);
			node *res = *ptr;
			if (res != nullptr) {
				if (res->sibling != nullptr) {
					node *nxt = res->sibling->sibling;
					delete res->sibling;
					res->sibling = nxt;
					res->dup_count -= 1;
				} else {
					*ptr = res->next;
					--size_;
					delete res;
					res = nullptr;
				}
			}
		}
		void remove_all(const std::string &key, uint32_t hash)
		{
			node **ptr = find_(key, hash);
			node *res = *ptr;
			if (res != nullptr) {
				if (res->sibling != nullptr) {
					node *root = res->sibling;
					while (root != nullptr) {
						node *nxt = root->sibling;
						delete root;
						root = nxt;
						res->dup_count -= 1;
					}
				}
				*ptr = res->next;
				--size_;
				delete res;
				res = nullptr;
			}
		}
		iterator search(const std::string &key, uint32_t hash)
		{
			return iterator(*find_(key, hash));
		}
		iterator search(const std::string &key)
		{
			uint32_t hash = murmurhash2(key.data(), key.size(), 0);
			return iterator(this->search(key, hash));
		}

	private:
		bool allow_multi_;
		uint32_t size_;
		uint32_t limit_;
		node **list_;
		void resize_()
		{
			uint32_t new_len = 4;
			while (new_len < size_)
				new_len *= 2;
			node **new_list = new node *[new_len];
			std::memset(new_list, 0, sizeof(new_list[0]) * new_len);
			uint32_t count = 0;
			for (uint32_t i = 0; i < limit_; ++i) {
				node *h = list_[i];
				while (h != nullptr) {
					node *next = h->next;
					uint32_t hash = h->hash;
					node **ptr =
						&new_list[hash & (new_len - 1)];
					h->next = *ptr;
					*ptr = h;
					h = next;
					count += 1;
				}
			}
			assert(size_ == count);
			delete[] list_;
			list_ = new_list;
			limit_ = new_len;
		}
		node **find_(const std::string &key, uint32_t hash)
		{
			node **ptr = &list_[hash & (limit_ - 1)];
			while (*ptr != nullptr &&
			       ((*ptr)->hash != hash || key != (*ptr)->key)) {
				ptr = &(*ptr)->next;
			}
			return ptr;
		}
	};
}
}

#endif
