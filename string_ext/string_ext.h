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
#include <regex>

namespace nm
{
  class string_ext : public std::string
  {
      using Base = std::string;
      using std::string::const_iterator;
      struct null_t {};
    public:
      constexpr static null_t null{};
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

      string_ext& to_upper()
      {
        std::transform(begin(), end(), begin(), [](unsigned char x) {
          return std::toupper(x);
        });
        return *this;
      }

      string_ext& to_lower()
      {
        std::transform(begin(), end(), begin(), [](unsigned char x) {
          return std::tolower(x);
        });
        return *this;
      }

      bool is_upper() const
      {
        return check([](const_iterator& iter) {
          return std::isupper(*iter);
        });
      }

      bool is_lower() const
      {
        return check([](const_iterator& iter) {
          return std::islower(*iter);
        });
      }

      bool is_alpha() const
      {
        return check([](const_iterator& iter) {
          return std::isalpha(*iter);
        });
      }

      bool is_alnum() const
      {
        return check([](const_iterator& iter) {
          return std::isalnum(*iter);
        });
      }

      bool is_digit() const
      {
        return check([](const_iterator& iter) {
          return std::isdigit(*iter);
        });
      }

      bool is_space() const
      {
        return check([](const_iterator& iter) {
          return std::isspace(*iter);
        });
      }

      bool match(const std::regex& re)
      {
        return std::regex_match(*this, re);
      }

      bool match(const string_ext& pattern)
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

      template<typename... Args>
      bool extract(const std::regex& re, Args&&... args)
      {
        std::smatch res;
        if(std::regex_match(*this, res, re))
        {
          return this->apply(res, Parser(std::forward<Args>(args))...);
        }
        return false;
      }

      template<typename... Args>
      bool extract(const string_ext& pattern, Args&&... args)
      {
        try
        {
          std::regex re{pattern};
          return this->extract(re, std::forward<Args>(args)...);
        }
        catch(const std::regex_error&)
        {
          return false;
        }
      }

      string_ext join(const std::vector<string_ext>& seq) const
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

      string_ext& lstrip(const string_ext& chars = string_ext())
      {
        return strip_impl(L_STRIP, chars);
      }

      string_ext& rstrip(const string_ext& chars = string_ext())
      {
        return strip_impl(R_STRIP, chars);
      }

      string_ext& strip(const string_ext& chars = string_ext())
      {
        strip_impl(L_STRIP, chars);
        return strip_impl(R_STRIP, chars);
      }

      string_ext& replace(const string_ext& oldstr,
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

      template<typename Container>
      void split(Container& res, const string_ext& sep = string_ext(),
                 long max = -1) const
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

    private:
      enum { L_STRIP = 0, R_STRIP };

      class Parser
      {
        public:
          template<typename T>
          Parser(T* x)
            : arg_(x), parser_([](const string_ext&, void*) {})
          {}
          Parser(void*) = delete;
          Parser(null_t)
            : arg_(nullptr), parser_([](const string_ext&, void*) {})
          {}

          Parser(int* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<int*>(res) = std::stoi(str);
          })
          {}

          Parser(long* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<long*>(res) = std::stol(str);
          })
          {}

          Parser(long long* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<long long*>(res) = std::stoll(str);
          })
          {}

          Parser(unsigned* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            using std::stoul;
            *static_cast<unsigned*>(res) = static_cast<unsigned>(stoul(str));
          })
          {}

          Parser(unsigned long* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<unsigned long*>(res) = std::stoul(str);
          })
          {}

          Parser(unsigned long long* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<unsigned long long*>(res) = std::stoull(str);
          })
          {}

          Parser(float* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<float*>(res) = std::stof(str);
          })
          {}

          Parser(double* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<double*>(res) = std::stod(str);
          })
          {}

          Parser(long double* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<long double*>(res) = std::stold(str);
          })
          {}

          Parser(bool* x)
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

          Parser(Base* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<Base*>(res) = str;
          })
          {}

          Parser(string_ext* x)
            : arg_(x), parser_([](const string_ext& str, void* res) {
            *static_cast<string_ext*>(res) = str;
          })
          {}

          bool parse(const string_ext& s)
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

        private:
          void* arg_;
          std::function<void(const string_ext&, void*)> parser_;
      };

      template<typename C, typename... Args>
      bool apply(C& c, Args&&... args)
      {
        auto argc = sizeof...(args);
        Parser* argv[] = {&args...};
        for(size_t i = 1; i < c.size() && i < argc + 1; ++i)
        {
          if(!argv[i - 1]->parse(c[i].str()))
            return false;
        }
        return true;
      }

      bool check(std::function<bool(const_iterator&)> pred) const
      {
        for(auto iter = cbegin(); iter != cend(); ++iter)
        {
          if(!pred(iter))
            return false;
        }
        return true;
      }

      string_ext& strip_impl(int type, const string_ext& chars)
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
  };
}

#endif // STRING_EXT_H_
