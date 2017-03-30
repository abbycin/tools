/*********************************************************
          File Name:clp.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Wed 29 Mar 2017 07:03:38 PM CST
**********************************************************/

#ifndef CLP
#define CLP

#include <string>
#include <unordered_map>
#include <optional>
#include <stdexcept>
#include <list>
#include <set>
#include <type_traits>

namespace nm
{
  class Clp
  {
    template<typename T, typename U> struct strict_same
    {
      constexpr static bool value = false;
    };
    template<typename T> struct strict_same<T, T>
    {
      constexpr static bool value = true;
    };

    public:
      enum struct Option : int
      {
        SHORT = 0,
        LONG
      };

      Clp(int argc, char* argv[]) noexcept
        : map_{}, cmd_{}, c_{}
      {
        for(int i = 1; i < argc; ++i)
          cmd_.push_back(argv[i]);
      }
      
      Clp(const Clp&) = delete;

      Clp(Clp&& rhs) noexcept
        : map_{std::move(rhs.map_)},
          cmd_{std::move(rhs.cmd_)},
          none_{std::move(rhs.none_)},
          c_{std::move(rhs.c_)},
          msg_{std::move(rhs.msg_)}
      {}

      Clp& operator= (const Clp&) = delete;

      ~Clp() noexcept {}

      Clp& parse(Option op = Option::SHORT) 
      {
        msg_.clear();
        switch(op)
        {
          case Option::SHORT:
            parse("-");
            break;
          case Option::LONG:
            parse("--");
            break;
        }
        return *this;
      }

      bool ok() const noexcept
      {
        return msg_.empty();
      }

      const std::string& msg() const noexcept
      {
        return msg_;
      }

      // implicitly construct optional.
      template<typename T>
      std::optional<T> get(const std::string& key) noexcept
      {
        static_assert(
            strict_same<bool, T>::value ||
            strict_same<int, T>::value ||
            strict_same<long, T>::value ||
            strict_same<long long, T>::value ||
            strict_same<unsigned int, T>::value ||
            strict_same<unsigned long, T>::value ||
            strict_same<unsigned long long, T>::value ||
            strict_same<float, T>::value ||
            strict_same<double, T>::value ||
            strict_same<long double, T>::value ||
            strict_same<std::string, T>::value,
            "\ntypename must be:\n"
            "boolean type,\n"
            "int type (signed or unsigned),\n"
            "float type,\n"
            "string type,\n"
            "WITH NO CV QUALIFIER.");

        msg_.clear();
        auto iter = map_.find(key);
        if(iter == map_.end())
        {
          msg_ = "no `" + key + "` in argument list";
          return {};
        }
        if constexpr(strict_same<bool, T>::value)
        {
          if(iter->second == "true")
            return true;
          else if(iter->second == "false")
            return false;
          else
            msg_ = "invalid argument";
          return {};
        }
        else if constexpr(strict_same<std::string, T>::value)
        {
          return iter->second;
        }
        else
        {
          T res = get_value<T>(iter->second);
          if(ok())
            return res;
          return {};
        }
      }

      const std::set<std::string>& none() const noexcept
      {
        return none_;
      }

      bool contain(const std::string& key) const noexcept
      {
        return c_.find(key) != c_.end();
      }

    private:
      std::unordered_map<std::string, std::string> map_;
      std::list<std::string> cmd_;
      std::set<std::string> none_;
      std::set<std::string> c_;
      std::string msg_;

      void parse(const std::string& delim)
      {
        std::string tmp{};
        bool has_key{false};
        msg_ = "invalid option";
        for(auto iter = cmd_.begin(); iter != cmd_.end(); ++iter)
        {
          has_key = false;
          if(*iter == "-")
          {
            return;
          }
          // get key
          if(iter->find_first_of(delim) != iter->npos && *iter != "--")
          {
            tmp = iter->substr(delim.length());
            has_key = true;
            ++iter;
          }
          if(iter == cmd_.end())
          {
            c_.emplace(tmp);
            break;
          }
          else if(*iter == "-")
          {
            return;
          }
          // get escape
          if(*iter == "--")
          {
            if(!has_key)
            {
              return;
            }
            if(++iter != cmd_.end())
            {
              map_.insert({tmp, *iter});
              c_.emplace(tmp);
            }
            else
            {
              // treat as single option
              c_.emplace(tmp);
              break;
            }
          }
          // no value
          else if(iter->find_first_of(delim) != iter->npos)
          {
            c_.emplace(tmp);
          }
          else if(has_key)
          {
            map_.insert({tmp, *iter});
            c_.emplace(tmp);
          }
          else
          {
            none_.emplace(*iter);
          }
        }
        msg_.clear();
      }

      template<typename T>
      T get_value(const std::string& str) noexcept
      {
        try
        {
          if constexpr(std::is_floating_point<T>::value)
            return static_cast<T>(std::stold(str));
          else if constexpr(std::is_signed<T>::value)
            return static_cast<T>(std::stoll(str));
          else
            return static_cast<T>(std::stoull(str));
        }
        catch(const std::out_of_range&)
        {
          msg_ = "error: out of range";
        }
        catch(const std::invalid_argument&)
        {
          msg_ = "error: invalid argument";
        }
        return {};
      }
  };
}

#endif // CLP
