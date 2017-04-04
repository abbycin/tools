/*********************************************************
          File Name: string_ext.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Mon 03 Apr 2017 11:31:20 AM CST
**********************************************************/

#ifndef STRING_EXT_H_
#define STRING_EXT_H_

#include <string>
#include <vector>
#include <limits>
#include <functional>

namespace nm
{
  class string_ext : public std::string
  {
      using Base = std::string;
    public:
      string_ext()
        : Base()
      {}

      string_ext(size_type count, char ch)
        : Base(count, ch)
      {}

      string_ext(const string_ext& other, size_type pos)
        : Base(other, pos)
      {}

      string_ext(const string_ext& other, size_type pos, size_type count)
        : Base(other, pos, count)
      {}

      string_ext(char* s, size_type count)
        : Base(s, count)
      {}

      string_ext(const char* s)
        : Base(s)
      {}

      template<typename InputIt>
      string_ext(InputIt first, InputIt last)
        : Base(first, last)
      {}

      string_ext(const string_ext& other)
        : Base(other)
      {}

      string_ext(string_ext&& other)
        : Base(std::move(other))
      {}

      string_ext(std::initializer_list<char> il)
        : Base(il)
      {}

      string_ext(const Base& rhs)
        : Base(rhs)
      {}

      string_ext(Base&& rhs)
        : Base(std::move(rhs))
      {}

      string_ext& operator= (const string_ext& rhs)
      {
        if(this != &rhs)
          Base::operator= (rhs);
        return *this;
      }

      string_ext& operator= (string_ext&& rhs)
      {
        if(this != &rhs)
          Base::operator= (std::move(rhs));
        return *this;
      }

      string_ext& to_upper();

      string_ext& to_lower();

      bool is_upper() const;

      bool is_lower() const;

      bool is_alpha() const;

      bool is_alnum() const;

      bool is_digit() const;

      bool is_space() const;

      string_ext join(const std::vector<string_ext>& seq) const;

      string_ext& lstrip(const string_ext& chars = string_ext());

      string_ext& rstrip(const string_ext& chars = string_ext());

      string_ext& strip(const string_ext& chars = string_ext());

      string_ext& replace(const string_ext& oldstr,
          const string_ext& newstr, long count = -1);

      void split(std::vector<string_ext>& res,
          const string_ext& sep = string_ext(), long max = -1) const;

    private:
      enum { L_STRIP = 0, R_STRIP};

      bool check(std::function<bool(const_iterator&)> pred) const;

      string_ext& strip_impl(int, const string_ext& chars);
  };
}

#endif // STRING_EXT_H_

