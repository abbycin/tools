/*********************************************************
          File Name: string_ext.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Mon 03 Apr 2017 12:55:18 PM CST
**********************************************************/

#include "string_ext.h"
#include <algorithm>
#include <cctype>

namespace nm
{
  bool string_ext::check(std::function<bool(iterator&)> pred)
  {
    for(auto iter = begin(); iter != end(); ++iter)
    {
      if(!pred(iter))
        return false;
    }
    return true;
  }

  string_ext& string_ext::strip_impl(int type, const string_ext& chars)
  {
    auto chars_len = chars.length();
    if(chars_len == 0)
    {
      if(type == L_STRIP)
      {
        auto iter = begin();
        while(iter != end() && std::isspace(*iter))
        {
          ++iter;
        }
        this->erase(0, iter - begin());
      }
      else
      {
        auto iter = rbegin();
        while(iter != rend() && std::isspace(*iter))
        {
          ++iter;
        }
        this->erase(rend() - iter);
      }
    }
    else
    {
      if(type == L_STRIP)
      {
        while(this->substr(0, chars_len) == chars)
        {
          this->erase(0, chars_len);
        }
      }
      else
      {
        while(this->size() > chars_len && 
            this->substr(this->size() - chars_len, chars_len) == chars)
        {
          this->erase(this->size() - chars_len);
        }
      }
    }
    return *this;
  }

  string_ext& string_ext::to_upper()
  {
    std::transform(begin(), end(), begin(), [](unsigned char x) {
      return std::toupper(x);
    });
    return *this;
  }

  string_ext& string_ext::to_lower()
  {
    std::transform(begin(), end(), begin(), [](unsigned char x) {
      return std::tolower(x);
    });
    return *this;
  }

  bool string_ext::is_upper()
  {
    return check([](iterator& iter) {
      return std::isupper(*iter);
    });
  }

  bool string_ext::is_lower()
  {
    return check([](iterator& iter) {
      return std::islower(*iter);
    });
  }

  bool string_ext::is_alpha()
  {
    return check([](iterator& iter) {
      return std::isalpha(*iter);
    });
  }

  bool string_ext::is_alnum()
  {
    return check([](iterator& iter) {
      return std::isalnum(*iter);
    });
  }

  bool string_ext::is_digit()
  {
    return check([](iterator& iter) {
      return std::isdigit(*iter);
    });
  }

  bool string_ext::is_space()
  {
    return check([](iterator& iter) {
      return std::isspace(*iter);
    });
  }

  string_ext string_ext::join(const std::vector<string_ext>& seq)
  {
    std::vector<string_ext>::size_type len = seq.size(), i = 0;
    if(len == 0)
      return {};
    if(len == 1)
      return seq[0];
    string_ext res;
    for(i = 1; i < len; ++i)
    {
      res += *this + seq[i];
    }
    return res;
  }

  string_ext& string_ext::lstrip(const string_ext& chars)
  {
    return strip_impl(L_STRIP, chars);
  }

  string_ext& string_ext::rstrip(const string_ext& chars)
  {
    return strip_impl(R_STRIP, chars);
  }

  string_ext& string_ext::strip(const string_ext& chars)
  {
    strip_impl(L_STRIP, chars);
    return strip_impl(R_STRIP, chars);
  }

  string_ext& string_ext::replace(const string_ext& oldstr,
                                  const string_ext& newstr,
                                  long count)
  {
    auto pos = this->find(oldstr);
    if(pos == npos)
      return *this;
    auto old_len = oldstr.size();
    std::function<bool()> pred{};
    if(count <= 0)
      pred = [&pos, this] { return pos != npos; };
    else
      pred = [&count, &pos, this] { return count-- > 0 && pos != npos; };

    while(pred())
    {
      Base::replace(pos, old_len, newstr);
      pos = Base::find(oldstr);
    }
    return *this;
  }

  void string_ext::split(std::vector<string_ext>& res,
                         const string_ext& sep, long max)
  {
    if(max < 0)
      max = std::numeric_limits<long>::max();

    auto sep_len = sep.size();
    auto cur = begin();
    auto iter = cur;
    if(sep_len == 0)
    {
      for(; max > 0 && iter != end(); ++iter)
      {
        if(std::isspace(*iter))
        {
          res.emplace_back(this->substr(cur - begin(), iter - cur));
          cur = iter;
          max -= 1;
        }
      }
    }
    else
    {
      while(max > 0 && iter != end())
      {
        if(this->substr(iter - begin(), sep_len) == sep)
        {
          res.emplace_back(this->substr(cur - begin(), iter - cur));
          iter += sep_len;
          cur = iter;
          max -= 1;
        }
        else
        {
          ++iter;
        }
      }
    }
    res.emplace_back(this->substr(cur - begin()));
  }
}