/*********************************************************
          File Name: string_view.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Wed 26 Jun 2019 06:15:56 PM CST
**********************************************************/

#ifndef STRING_VIEW_H_
#define STRING_VIEW_H_

#include <algorithm>
#include <iterator>
#include <numeric>
#include <string>
#include <cstring>

namespace nm
{
  template<typename StringType>
  class basic_string_view
  {
  public:
    // member types
    using value_type = typename StringType::value_type;
    using size_type = size_t;
    using const_iterator = const value_type*;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    constexpr static size_t uchar_max = std::numeric_limits<unsigned char>::max();

    // constants
    constexpr static size_type npos = size_type(-1);

    // member functions
    constexpr basic_string_view()
      : data_{ nullptr }, len_{ 0 }
    {}

    basic_string_view(const value_type* data)
      : data_{ data }, len_{ data ? StringType::traits_type::length(data) : 0 }
    {}

    constexpr basic_string_view(const value_type* data, size_type len)
      : data_{ data }, len_{ len }
    {}

    constexpr basic_string_view(const basic_string_view& rhs)
      : data_{ rhs.data_ }, len_{ rhs.len_ }
    {}

    basic_string_view& operator= (basic_string_view&& rhs) noexcept
    {
      if(this != &rhs)
      {
        data_ = rhs.data_;
        len_ = rhs.len_;
        rhs.clear();
      }
      return *this;
    }

    basic_string_view& operator= (const basic_string_view& rhs)
    {
      if(this != &rhs)
      {
        data_ = rhs.data_;
        len_ = rhs.len_;
      }
      return *this;
    }

    void assign(const value_type* data)
    {
      data_ = data;
      len_ = data ? StringType::traits_type::length(data) : 0;
    }

    void assign(const value_type* data, size_type len)
    {
      data_ = data;
      len_ = len;
    }

    // element access
    value_type at(size_type i) const
    {
      if(i >= len_)
      {
        throw std::out_of_range("index out of range");
      }
      return data_[i];
    }

    value_type operator[] (size_type i) const
    {
      return data_[i];
    }

    const value_type* data() const
    {
      return data_;
    }

    const value_type* c_str() const
    {
      return data_;
    }

    // iterators
    const_iterator begin() const
    {
      return data_;
    }

    const_iterator end() const
    {
      return data_ + len_;
    }

    const_reverse_iterator rbegin() const
    {
      return const_reverse_iterator(data_ + len_);
    }

    const_reverse_iterator rend() const
    {
      return const_reverse_iterator(data_);
    }

    // capacity
    bool empty() const
    {
      return len_ == 0;
    }

    size_type size() const
    {
      return len_;
    }

    size_type length() const
    {
      return len_;
    }

    size_type max_size() const
    {
      return len_;
    }

    size_type capacity() const
    {
      return len_;
    }

    // operations
    void clear()
    {
      data_ = nullptr;
      len_ = 0;
    }

    int compare(const basic_string_view& x) const
    {
      int r = basic_string_view<StringType>::cmp(data_, x.data_, std::min(len_, x.len_));
      if(r == 0)
      {
        if(len_ < x.len_)
        {
          r = -1;
        }
        else if(len_ > x.len_)
        {
          r = 1;
        }
      }
      return r;
    }

    void remove_prefix(size_type n)
    {
      data_ += n;
      len_ -= n;
    }

    void remove_suffix(size_type n)
    {
      len_ -= n;
    }

    bool starts_with(const basic_string_view& x) const
    {
      return ((len_ >= x.len_) && (cmp(data_, x.data_, x.len_) == 0));
    }

    bool ends_with(const basic_string_view& x) const
    {
      return ((len_ >= x.len_) && (cmp(data_ + (len_ - x.len_), x.data_, x.len_) == 0));
    }

    basic_string_view substr(size_type pos, size_type size) const
    {
      if(pos > this->size())
      {
        pos = this->size();
      }
      if(size > this->size() - pos)
      {
        size = this->size() - pos;
      }
      return { data_ + pos, size };
    }

    size_type copy(value_type* s, size_type count, size_type pos = 0) const
    {
      auto r = std::min(this->size() - pos, count);
      std::memcpy(s, data_ + pos, r);
      return r;
    }

    // search
    size_type find(const basic_string_view& x, size_type pos = 0) const
    {
      if(pos > x.size())
      {
        return npos;
      }
      auto r = std::search(this->begin() + pos, this->end(), x.begin(), x.end());
      const size_type rpos = static_cast<size_type>(r - this->begin());
      return rpos + x.size() <= this->size() ? rpos : npos;
    }

    size_type find(char x, size_type pos = 0) const
    {
      if(pos > this->size())
      {
        return npos;
      }
      auto r = std::find(this->begin() + pos, this->end(), x);
      return r != this->end() ? static_cast<size_type>(r - this->begin()) : npos;
    }

    size_type rfind(const basic_string_view& x, size_type pos = npos) const
    {
      if(this->size() < x.size())
      {
        return npos;
      }
      if(x.empty())
      {
        return std::min(this->size(), pos);
      }
      auto last = this->begin() + std::min(this->size() - x.size(), pos) + x.size();
      auto r = std::find_end(this->begin(), last, x.begin(), x.end());
      return r != last ? static_cast<size_type>(r - this->begin()) : npos;
    }

    size_type rfind(char x, size_type pos = npos) const
    {
      if(this->size() == 0)
      {
        return npos;
      }
      for(size_type i = std::min(pos, this->size() - 1); i > 0; --i)
      {
        if(this->data()[i] == x)
        {
          return i;
        }
      }
      return npos;
    }

    size_type find_first_of(const basic_string_view& x, size_type pos = 0) const
    {
      if(this->size() == 0 || x.size() == 0)
      {
        return npos;
      }
      if(x.size() == 1)
      {
        return this->find(x.data()[0], pos);
      }
      bool lookup[uchar_max + 1] = { false };
      build_table(x, lookup);
      for(size_type i = pos; i < this->size(); ++i)
      {
        if(lookup[static_cast<unsigned char>(this->data()[i])])
        {
          return i;
        }
      }
      return npos;
    }

    size_type find_first_not_of(const basic_string_view& x, size_type pos = 0) const
    {
      if(this->size() == 0)
      {
        return npos;
      }
      if(x.size() == 0)
      {
        return 0;
      }
      if(x.size() == 1)
      {
        return this->find_first_not_of(x.data()[0], pos);
      }
      bool lookup[uchar_max + 1] = { false };
      build_table(x, lookup);
      for(size_type i = pos; i < this->size(); ++i)
      {
        if(!lookup[static_cast<unsigned char>(this->data()[i])])
        {
          return i;
        }
      }
      return npos;
    }

    size_type find_first_not_of(char x, size_type pos = 0) const
    {
      if(this->size() == 0)
      {
        return npos;
      }
      for(; pos < this->size(); ++pos)
      {
        if(this->data()[pos] != x)
        {
          return pos;
        }
      }
      return npos;
    }

    size_type find_last_of(const basic_string_view& x, size_type pos = npos) const
    {
      if(this->size() == 0 || x.size() == 0)
      {
        return npos;
      }
      if(x.size() == 1)
      {
        return this->rfind(x.data()[0], pos);
      }
      bool lookup[uchar_max + 1] = { false };
      build_table(x, lookup);
      for(size_type i = std::min(pos, this->size() - 1); i > 0; --i)
      {
        if(lookup[static_cast<unsigned char>(this->data()[i])])
        {
          return i;
        }
      }
      return npos;
    }

    size_type find_last_not_of(const basic_string_view& x, size_type pos = npos) const
    {
      if(this->size() == 0)
      {
        return npos;
      }
      auto i = std::min(pos, this->size() - 1);
      if(x.size() == 0)
      {
        return i;
      }
      if(x.size() == 1)
      {
        return this->find_last_not_of(x.data()[0], pos);
      }
      bool lookup[uchar_max + 1] = { false };
      build_table(x, lookup);
      for(; i > 0; --i)
      {
        if(!lookup[static_cast<unsigned char>(this->data()[i])])
        {
          return i;
        }
      }
      return npos;
    }

    size_type find_last_not_of(char x, size_type pos = npos) const
    {
      if(this->size() == 0)
      {
        return npos;
      }
      for(size_type i = std::min(pos, this->size() - 1); i > 0; --i)
      {
        if(this->data()[i] != x)
        {
          return i;
        }
      }
      return npos;
    }

    std::string to_string()
    {
      return { data_, len_ };
    }

    static int cmp(const value_type* l, const value_type* r, size_type n)
    {
      return StringType::traits_type::compare(l, r, n);
    }

  protected:
    const value_type* data_;
    size_type len_;

    // table[uchar_max + 1]
    static void build_table(const basic_string_view& wanted, bool* table)
    {
      auto size = wanted.size();
      auto data = wanted.data();
      for(size_type i = 0; i < size; ++i)
      {
        table[static_cast<unsigned char>(data[i])] = true;
      }
    }
  };

  using string_view = basic_string_view<std::string>;

  bool operator== (const string_view& x, const string_view& y)
  {
    auto r = string_view::cmp(x.data(), y.data(), std::min(x.size(), y.size()));
    return r == 0 && x.size() == y.size();
  }

  inline bool operator!= (const string_view& x, const string_view& y)
  {
    return !(x == y);
  }

  inline bool operator< (const string_view& x, const string_view& y)
  {
    auto r = string_view::cmp(x.data(), y.data(), (x.size() < y.size() ? x.size() : y.size()));
    return ((r < 0) || ((r == 0) && (x.size() < y.size())));
  }

  inline bool operator> (const string_view& x, const string_view& y)
  {
    return y < x;
  }

  inline bool operator<= (const string_view& x, const string_view& y)
  {
    return !(x > y);
  }

  inline bool operator>= (const string_view& x, const string_view& y)
  {
    return !(x < y);
  }

  inline std::ostream& operator<< (std::ostream& os, const string_view& v)
  {
    os.write(v.data(), v.size());
    return os;
  }

  constexpr string_view operator""_sv(const string_view::value_type* data, string_view::size_type len)
  {
    return { data, len };
  }
}

#endif // STRING_VIEW_H_
