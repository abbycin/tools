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
  bool string_ext::check(std::function<bool(const_iterator&)> pred) const
  {
    for(auto iter = cbegin(); iter != cend(); ++iter)
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

  bool string_ext::is_upper() const
  {
    return check([](const_iterator& iter) {
      return std::isupper(*iter);
    });
  }

  bool string_ext::is_lower() const
  {
    return check([](const_iterator& iter) {
      return std::islower(*iter);
    });
  }

  bool string_ext::is_alpha() const
  {
    return check([](const_iterator& iter) {
      return std::isalpha(*iter);
    });
  }

  bool string_ext::is_alnum() const
  {
    return check([](const_iterator& iter) {
      return std::isalnum(*iter);
    });
  }

  bool string_ext::is_digit() const
  {
    return check([](const_iterator& iter) {
      return std::isdigit(*iter);
    });
  }

  bool string_ext::is_space() const
  {
    return check([](const_iterator& iter) {
      return std::isspace(*iter);
    });
  }

  bool string_ext::match(const std::regex& re)
  {
    return std::regex_match(*this, re);
  }

  bool string_ext::match(const string_ext& pattern)
  {
    try
    {
      std::regex re{pattern};
      return this->match(re);
    }
    catch(const std::regex_error&)
    {
      return false;
    }
  }

  string_ext string_ext::join(const std::vector<string_ext>& seq) const
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
                         const string_ext& sep, long max) const
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

  string_ext::Parser::Parser(void* x)
    : arg_(x), parser_([](const string_ext&, void*) {})
  {}

  string_ext::Parser::Parser(int* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<int*>(res) = std::stoi(str);
      })
  {}

  string_ext::Parser::Parser(long* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<long*>(res) = std::stol(str);
      })
  {}

  string_ext::Parser::Parser(long long* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<long long*>(res) = std::stoll(str);
      })
  {}

  string_ext::Parser::Parser(unsigned* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<unsigned*>(res) = std::stoul(str);
      })
  {}

  string_ext::Parser::Parser(unsigned long* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<unsigned long*>(res) = std::stoul(str);
      })
  {}

  string_ext::Parser::Parser(unsigned long long* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<unsigned long long*>(res) = std::stoull(str);
      })
  {}

  string_ext::Parser::Parser(float* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<float*>(res) = std::stof(str);
      })
  {}

  string_ext::Parser::Parser(double* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<double*>(res) = std::stod(str);
      })
  {}

  string_ext::Parser::Parser(long double* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<long double*>(res) = std::stold(str);
      })
  {}

  string_ext::Parser::Parser(bool* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        auto& r = *static_cast<bool*>(res);
        if(str == "true")
          r = true;
        else if(str == "false")
          r = false;
        else
          throw std::logic_error("invalid argument");
      })
  {}

  string_ext::Parser::Parser(Base* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<Base*>(res) = str;
      })
  {}

  string_ext::Parser::Parser(string_ext* x)
    : arg_(x), parser_([](const string_ext& str, void* res) {
        *static_cast<string_ext*>(res) = str;
      })
  {}

  bool string_ext::Parser::parse(const string_ext& s)
  {
    try
    {
      parser_(s, arg_);
    }
    catch(...)
    {
      return false;
    }
    return true;
  }
}
