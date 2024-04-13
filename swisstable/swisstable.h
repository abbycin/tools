// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Author: Abby Cin
 * Mail: abbytsing@gmail.com
 * Create Time: 2023-09-21 16:24:38
 */

#pragma once

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <emmintrin.h>
#include <memory>
#include <type_traits>

namespace nm
{
namespace detail
{
	template<typename T>
	concept Hashable = requires(T t) {
		{ t.data() } -> std::convertible_to<const void *>;
		{ t.size() } -> std::convertible_to<uint64_t>;
	};
}

struct HashFn {
	static uint64_t murmur(const void *key, uint64_t len)
	{
		const uint64_t m = 0xc6a4a7935bd1e995UL;
		const int r = 47;
		const uint64_t seed = 0x1f0d3804ul;

		uint64_t h = seed ^ (len * m);

		const uint64_t *data = (const uint64_t *)key;
		const uint64_t *end = data + (len / 8);

		while (data != end) {
			uint64_t k = *data++;

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}

		const unsigned char *data2 = (const unsigned char *)data;

		switch (len & 7) {
		case 7:
			h ^= ((uint64_t)data2[6]) << 48;
			[[fallthrough]];
		case 6:
			h ^= ((uint64_t)data2[5]) << 40;
			[[fallthrough]];
		case 5:
			h ^= ((uint64_t)data2[4]) << 32;
			[[fallthrough]];
		case 4:
			h ^= ((uint64_t)data2[3]) << 24;
			[[fallthrough]];
		case 3:
			h ^= ((uint64_t)data2[2]) << 16;
			[[fallthrough]];
		case 2:
			h ^= ((uint64_t)data2[1]) << 8;
			[[fallthrough]];
		case 1:
			h ^= ((uint64_t)data2[0]);
			h *= m;
		};

		h ^= h >> r;
		h *= m;
		h ^= h >> r;

		return h;
	}
};

struct SwissHash {
	template<detail::Hashable Key>
	static uint64_t hash(const Key &key)
	{
		return HashFn::murmur(key.data(), key.size());
	}

	template<std::integral Key>
	static uint64_t hash(const Key &key)
	{
		return HashFn::murmur(&key, sizeof(Key));
	}

	template<typename Key,
		 typename = std::enable_if_t<std::is_pointer_v<Key>, int>>
	static uint64_t hash(const Key &key)
	{
		return HashFn::murmur(key, sizeof(void *));
	}
};

struct SwissEq {
	template<std::equality_comparable Key>
	static bool eq(const Key &lhs, const Key &rhs)
	{
		return lhs == rhs;
	}

	template<std::equality_comparable T, std::equality_comparable U>
	static bool eq(const T &lhs, const U &rhs)
	{
		return lhs == rhs;
	}
};

namespace detail
{
	template<typename Policy, typename Hash, typename Eq>
	concept key_constraint = requires(const typename Policy::key_type &k) {
		{ Hash::hash(k) } -> std::convertible_to<uint64_t>;
		{ Eq::eq(k, k) } -> std::convertible_to<bool>;
	};

	// NOTE: prefer moving to copying
	template<typename Policy,
		 typename Hash = SwissHash,
		 typename Eq = SwissEq>
		requires key_constraint<Policy, Hash, Eq> &&
		std::move_constructible<typename Policy::key_type> &&
		std::copy_constructible<typename Policy::key_type>
	class Swiss {
		enum ctrl_t : int8_t {
			k_empty = -128,
			k_deleted = -2,
			k_sentinel = -1,
		};

	public:
		using key_type = Policy::key_type;
		using value_type = Policy::value_type;
		struct iterator {
			friend Swiss;

			value_type &operator*()
			{
				return *slot;
			}

			explicit operator bool()
			{
				return ctrl != nullptr;
			}

			value_type *operator->()
			{
				return slot;
			}

			iterator &operator++()
			{
				ctrl += 1;
				assert(slot);
				slot += 1;
				next();
				return *this;
			}

			friend bool operator!=(const iterator &lhs,
					       const iterator &rhs)
			{
				return lhs.ctrl != rhs.ctrl;
			}

			friend bool operator==(const iterator &lhs,
					       const iterator &rhs)
			{
				return lhs.ctrl == rhs.ctrl;
			}

		private:
			iterator(ctrl_t *c, value_type *s)
				: ctrl { c }, slot { s }
			{
			}

			void next()
			{
				while (is_empty_or_delete(*ctrl)) {
					auto shift =
						group { ctrl }
							.ctl_empty_or_delete();
					ctrl += shift;
					slot += shift;
				}
				if (*ctrl == k_sentinel) {
					slot = nullptr;
					ctrl = nullptr;
				}
			}

			ctrl_t *ctrl;
			value_type *slot;
		};

		explicit Swiss(uint64_t size = k_width * 2)
			: elems_ { 0 }
			, groups_ { calc_groups(size) }
			, cap_ { calc_cap(groups_) }
		{
			assert(is_power_of_2(groups_));
			new_table(cap_, &ctrl_, &slot_);
		}

		Swiss(const Swiss &) = delete;

		Swiss(Swiss &&s) noexcept
			: elems_ { 0 }
			, groups_ { 0 }
			, cap_ { 0 }
			, ctrl_ { 0 }
			, slot_ { 0 }
		{
			*this = std::move(s);
		}

		~Swiss()
		{
			if (ctrl_)
				clear();
			free(ctrl_);
			ctrl_ = nullptr;
		}

		Swiss &operator=(const Swiss &) = delete;

		Swiss &operator=(Swiss &&s) noexcept
		{
			if (this != &s) {
				std::swap(elems_, s.elems_);
				std::swap(groups_, s.groups_);
				std::swap(cap_, s.cap_);
				std::swap(ctrl_, s.ctrl_);
				std::swap(slot_, s.slot_);
			}
			return *this;
		}

		double load_factor() const
		{
			return static_cast<double>(elems_) / cap_;
		}

		bool set_max_load_factor(double x)
		{
			if (x > 1.0 ||
			    std::fabs(x - 1.0) <
				    std::numeric_limits<double>::epsilon())
				return false;
			load_factor_ = x;
			return true;
		}

		double max_load_factor() const
		{
			return load_factor_;
		}

		template<typename T = key_type>
		iterator find(const T &key) const
		{
			uint64_t hash = Hash::hash(key);
			auto pattern = _mm_set1_epi8(H2(hash));
			prober seq { H1(hash), cap_ };
			matcher m { 0 };

			prefetch(seq.offset());
			while (true) {
				group g { ctrl_ + seq.offset() };
				m = g.match(pattern);

				while (m) {
					auto i = seq.offset(*m);
					if (Eq::eq(Policy::key(slot_[i]), key))
						return iterator_at(i);
					++m;
				}
				if (g.match_empty())
					return end();
				seq.next();
			}
		}

		template<typename T>
		bool contains(const T &key) const
		{
			return find(key) != end();
		}

		iterator insert(const value_type &key)
		{
			auto v = key;
			return try_insert(std::move(v));
		}

		template<typename... Args>
		iterator emplace(Args &&...args)
		{
			return try_insert(
				value_type { std::forward<Args>(args)... });
		}

		template<typename T>
		bool erase(const T &key)
		{
			auto iter = find(key);

			if (iter == end())
				return false;
			invalidate(iter.ctrl);
			elems_ -= 1;
			return true;
		}

		iterator &erase(iterator &it)
		{
			assert(it != end());
			invalidate(it.ctrl);
			elems_ -= 1;
			return ++it;
		}

		void reserve(size_t size)
		{
			assert(cap_ != 0);
			if (size > cap_) {
				Swiss tmp { size };
				for (auto &i : *this)
					tmp.insert(std::move(i));
				*this = std::move(tmp);
			}
		}

		uint64_t size() const
		{
			return elems_;
		}

		uint64_t bucket_count() const
		{
			return cap_;
		}

		uint64_t cap() const
		{
			return cap_;
		}

		void clear()
		{
			assert(ctrl_);
			for (auto iter = begin(); iter != end(); ++iter)
				invalidate(iter.ctrl);
			elems_ = 0;
		}

		iterator begin() const
		{
			auto it = iterator_at(0);
			it.next();
			return it;
		}

		iterator end() const
		{
			return { nullptr, nullptr };
		}

	private:
		enum {
			k_width = 16,
			k_clone = k_width - 1, // adapt float window
		};
		uint64_t elems_ = 0;
		double load_factor_ = 15.0 / static_cast<int>(k_width);
		uint64_t groups_;
		uint64_t cap_;
		ctrl_t *ctrl_;
		value_type *slot_;

		static constexpr bool is_empty_or_delete(ctrl_t ctrl)
		{
			return ctrl < k_sentinel;
		}
		class prober {

		public:
			prober(uint64_t h1, uint64_t mask)
				: offset_ { h1 & mask }, mask_ { mask }
			{
				assert(is_power_of_2(mask + 1));
			}

			void next()
			{
				group_ += k_width;
				offset_ += group_;
				offset_ = offset_ & mask_;
			}

			uint64_t offset() const
			{
				return offset_;
			}

			uint64_t offset(int i)
			{
				return (offset_ + i) & mask_;
			}

		private:
			uint64_t offset_;
			uint64_t mask_;
			uint64_t group_ = 0;
		};

		class matcher {
		public:
			matcher(int32_t x) : val_ { static_cast<uint32_t>(x) }
			{
			}

			explicit operator bool()
			{
				return val_ != 0;
			}

			matcher &operator++()
			{
				val_ &= (val_ - 1);
				return *this;
			}

			int operator*() const
			{
				assert(val_ != 0);
				return __builtin_ctz(val_);
			}

		private:
			uint32_t val_;
		};

		class group {
		public:
			group(const ctrl_t *ctrl)
				: ctrl_ { _mm_loadu_si128(
					  reinterpret_cast<const __m128i *>(
						  ctrl)) }
			{
			}

			matcher match(__m128i &p)
			{
				return { _mm_movemask_epi8(
					_mm_cmpeq_epi8(p, ctrl_)) };
			}

			matcher match_empty()
			{
				auto pattern = _mm_set1_epi8(k_empty);
				return match(pattern);
			}

			matcher match_empty_or_delete()
			{
				const __m128i p = _mm_set1_epi8(k_sentinel);
				return { _mm_movemask_epi8(
					_mm_cmpgt_epi8(p, ctrl_)) };
			}

			uint32_t ctl_empty_or_delete()
			{
				const __m128i p = _mm_set1_epi8(k_sentinel);
				return __builtin_ctz(static_cast<uint32_t>(
					_mm_movemask_epi8(
						_mm_cmpgt_epi8(p, ctrl_)) +
					1));
			}

		private:
			__m128i ctrl_;
		};

		static constexpr bool is_power_of_2(uint64_t x)
		{
			return x && !(x & (x - 1));
		}

		static constexpr uint64_t next_power_of_2(uint64_t x)
		{
			x -= 1;
			x |= x >> 1;
			x |= x >> 2;
			x |= x >> 4;
			x |= x >> 8;
			x |= x >> 16;
			x |= x >> 32;
			return x + 1;
		}

		static constexpr uint64_t calc_groups(uint64_t size)
		{
			if (size < 2 * k_width)
				size = 2 * k_width; // avoid corner case
			uint64_t x = size / k_width;
			return next_power_of_2(x);
		}

		static constexpr uint64_t calc_cap(uint64_t groups)
		{
			return groups * k_width - 1;
		}

		static constexpr uint64_t H1(uint64_t hash)
		{
			return hash >> 7;
		}

		static constexpr ctrl_t H2(uint64_t hash)
		{
			return static_cast<ctrl_t>(hash & 0x7f);
		}

		static constexpr uint64_t ctrl_bytes(uint64_t cap)
		{
			return cap + 1 + k_clone;
		}

		static constexpr uint64_t slot_align()
		{
			return std::max(alignof(value_type), sizeof(uint64_t));
		}

		static constexpr uint64_t slot_offset(uint64_t cap)
		{
			auto align = slot_align();
			return (ctrl_bytes(cap) + align - 1) & (~align + 1);
		}

		static constexpr uint64_t slot_bytes(uint64_t cap)
		{
			return slot_offset(cap) + cap * sizeof(value_type);
		}

		static constexpr void
		set_ctrl(ctrl_t *ctrl, uint64_t i, ctrl_t h2, uint64_t mask)
		{
			ctrl[i] = h2;
			// since sse can't round from end to begin
			// we extend begin to the end, see new_table、ctrl_bytes
			ctrl[((i - k_clone) & mask) + (k_clone & mask)] = h2;
		}

		static void
		new_table(uint64_t cap, ctrl_t **ctrl, value_type **slot)
		{
			static_assert(sizeof(ctrl_t) == 1);
			auto size = slot_bytes(cap) + ctrl_bytes(cap);
			*ctrl = reinterpret_cast<ctrl_t *>(malloc(size));
			*slot = reinterpret_cast<value_type *>(
				*ctrl + slot_offset(cap));
			std::memset(*ctrl, k_empty, ctrl_bytes(cap));
			(*ctrl)[cap] = k_sentinel;
		}

		auto iterator_at(uint64_t pos) const
		{
			return iterator { ctrl_ + pos, slot_ + pos };
		}

		void invalidate(const ctrl_t *item)
		{
			// NOTE: can't set to k_empty, since insert and
			// find rely on k_empty to stop search
			uint64_t offset = item - ctrl_;
			set_ctrl(ctrl_, offset, k_deleted, cap_);
			std::destroy_at(slot_ + offset);
		}

		void prefetch(uint64_t offset) const
		{
			__builtin_prefetch(ctrl_, 0, 1);
			__builtin_prefetch(ctrl_ + offset, 0, 3);
			__builtin_prefetch(slot_ + offset, 0, 3);
		}

		iterator try_insert(value_type &&v)
		{
			if (load_factor() > max_load_factor())
				reserve(calc_cap(groups_ * 2));
			const auto &key = Policy::key(v);
			auto hash = Hash::hash(key);
			auto h2 = H2(hash);
			auto pattern = _mm_set1_epi8(h2);
			prober seq { H1(hash), cap_ };

			while (true) {
				group g { ctrl_ + seq.offset() };
				auto m = g.match(pattern);

				while (m) {
					if (Eq::eq(Policy::key(slot_[seq.offset(
							   *m)]),
						   key))
						return end();
					++m;
				}
				m = g.match_empty_or_delete();
				if (m) {
					auto pos = seq.offset(*m);
					set_ctrl(ctrl_, pos, h2, cap_);
					std::construct_at(slot_ + pos,
							  std::move(v));
					elems_ += 1;
					return iterator_at(pos);
				}
				seq.next();
			}
		}
	};
}
} // namespace nm
