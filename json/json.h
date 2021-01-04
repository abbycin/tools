/***********************************************
        File Name: json.h
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 12/28/20 10:40 AM
***********************************************/

#ifndef JSON_H_
#define JSON_H_

#include <cassert>
#include <limits>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <sstream>
#include <string>
#include <vector>

#ifndef NDEBUG
#include <iostream>
struct debugger
{
  debugger(const char* s, int n) { std::cerr << s << ':' << n << " => "; }
  template<typename... Args>
  void print(Args&&... args)
  {
    ((std::cerr << args << ' '), ...);
    std::cerr << '\n';
  }
};
#define debug(...) debugger(__func__, __LINE__).print(__VA_ARGS__)

#else
#define debug(...)
#endif

namespace nm::json
{
  class JsonValue;
  struct null_t
  {
  };
  using number_t = double;
  using bool_t = bool;
  using string_t = std::string;
  using array_t = std::vector<JsonValue>;
  using object_t = std::map<std::string, JsonValue>;
  inline constexpr size_t max_depth = 50;

  namespace detail
  {
    template<typename T>
    struct nullable
    {
      nullable() : ok{false}, ch{} {}
      nullable(T&& c) : ok{true}, ch{std::move(c)} {}

      bool operator==(const nullable& r) { return ok == r.ok && ch == r.ch; }
      explicit operator bool() { return ok; }
      bool operator!=(const nullable& r) { return ok != r.ok || ch != r.ch; }
      T& operator*() { return ch; }
      T* operator->() { return &ch; }

    private:
      bool ok;
      T ch;
    };

    // union-like class
    struct value_type
    {
      value_type() : null_{}, type{invalid} {}
      ~value_type()
      {
        switch(type)
        {
        case value_type::string:
        {
          delete string_;
          break;
        }
        case value_type::object:
        {
          delete object_;
          break;
        }
        case value_type::array:
        {
          delete array_;
          break;
        }
        default:
          break;
        }
      }

      enum Category
      {
        null = 0,
        invalid,
        boolean,
        number,
        array,
        string,
        object
      };
      union
      {
        null_t null_;
        bool_t bool_;
        number_t number_;
        array_t* array_;
        string_t* string_;
        object_t* object_;
      };
      Category type;
    };

    template<typename T>
    struct type_map;
    template<>
    struct type_map<null_t>
    {
      static constexpr value_type::Category type = value_type::null;
    };
    template<>
    struct type_map<bool_t>
    {
      static constexpr value_type::Category type = value_type::boolean;
    };
    template<>
    struct type_map<number_t>
    {
      static constexpr value_type::Category type = value_type::number;
    };
    template<>
    struct type_map<array_t>
    {
      static constexpr value_type::Category type = value_type::array;
    };
    template<>
    struct type_map<string_t>
    {
      static constexpr value_type::Category type = value_type::string;
    };
    template<>
    struct type_map<object_t>
    {
      static constexpr value_type::Category type = value_type::object;
    };

    template<typename T>
    struct value_map;
    template<>
    struct value_map<null_t>
    {
      static constexpr null_t* value(value_type& v) { return &v.null_; }
    };
    template<>
    struct value_map<bool_t>
    {
      static constexpr bool_t* value(value_type& v) { return &v.bool_; }
    };
    template<>
    struct value_map<number_t>
    {
      static constexpr number_t* value(value_type& v) { return &v.number_; }
    };
    template<>
    struct value_map<array_t>
    {
      static constexpr array_t* value(value_type& v) { return v.array_; }
    };
    template<>
    struct value_map<string_t>
    {
      static constexpr string_t* value(value_type& v) { return v.string_; }
    };
    template<>
    struct value_map<object_t>
    {
      static constexpr object_t* value(value_type& v) { return v.object_; }
    };
  } // namespace detail

  class JsonValue final
  {
    using value_type = detail::value_type;

  public:
    JsonValue() : value_{new value_type{}}, trace_{} {}
    template<size_t N>
    JsonValue(const char (&a)[N]) : JsonValue{string_t{a, N}}
    {
    }
    // no explicit
    JsonValue(const string_t& s) : JsonValue{}
    {
      value_->string_ = new std::string{s};
      value_->type = value_type::string;
    }
    JsonValue(string_t&& s) : JsonValue{}
    {
      value_->string_ = new std::string{std::move(s)};
      value_->type = value_type::string;
    }
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, number_t>>>
    JsonValue(T d) : JsonValue{}
    {
      value_->number_ = (number_t)d;
      value_->type = value_type::number;
    }
    JsonValue(null_t n) : JsonValue{}
    {
      value_->null_ = n;
      value_->type = value_type::null;
    }
    JsonValue(bool_t b) : JsonValue{}
    {
      value_->bool_ = b;
      value_->type = value_type::boolean;
    }
    JsonValue(array_t&& a) : JsonValue{}
    {
#if 1
      value_->array_ = new array_t{};
      *value_->array_ = std::move(a);
#else
      // recursively call itself when using g++ (clang++ is ok)
      value_->data.array_ = new array_t{std::move(a)};
#endif
      value_->type = value_type::array;
    }
    // never use std::initializer_list<JsonValue> for array
    JsonValue(const std::initializer_list<std::pair<string_t, JsonValue>>& il) : JsonValue{}
    {
      value_->object_ = new object_t{il.begin(), il.end()};
      value_->type = value_type::object;
    }
    JsonValue(object_t&& o) : JsonValue{}
    {
      value_->object_ = new object_t{std::move(o)};
      value_->type = value_type::object;
    }

    JsonValue(JsonValue&& r) noexcept : value_{std::move(r.value_)}, trace_{std::move(r.trace_)} {}

    JsonValue& operator=(JsonValue&& r) noexcept
    {
      if(this != &r)
      {
        value_ = std::move(r.value_);
        trace_ = std::move(r.trace_);
      }
      return *this;
    }

    JsonValue(const JsonValue& r) : value_{r.value_}, trace_{r.trace_} {}

    JsonValue& operator=(const JsonValue& r)
    {
      if(this != &r)
      {
        value_ = r.value_;
        trace_ = r.trace_;
      }
      return *this;
    }

    ~JsonValue() = default;

    JsonValue& operator=(null_t)
    {
      value_->~value_type();
      value_->type = value_type::null;
      return *this;
    }

    JsonValue& operator=(bool_t b)
    {
      value_->~value_type();
      value_->bool_ = b;
      value_->type = value_type::boolean;
      return *this;
    }

    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, number_t>>>
    JsonValue& operator=(T t)
    {
      value_->~value_type();
      value_->number_ = t;
      value_->type = value_type::number;
      return *this;
    }

    template<size_t N>
    JsonValue& operator=(const char (&a)[N])
    {
      value_->~value_type();
      value_->string_ = new string_t{a, N};
      value_->type = value_type::string;
      return *this;
    }

    JsonValue& operator=(const string_t& s)
    {
      value_->~value_type();
      value_->string_ = new std::string{s};
      value_->type = value_type::string;
      return *this;
    }

    JsonValue& operator=(string_t&& s)
    {
      value_->~value_type();
      value_->string_ = new std::string{std::move(s)};
      value_->type = value_type::string;
      return *this;
    }

    JsonValue& operator=(const array_t& a)
    {
      value_->~value_type();
      value_->array_ = new array_t{};
      for(auto& x: a)
      {
        value_->array_->emplace_back(x.clone());
      }
      value_->type = value_type::array;
      return *this;
    }

    JsonValue& operator=(array_t&& a)
    {
      value_->~value_type();
      value_->array_ = new array_t{};
      *value_->array_ = std::move(a); // see JsonValue(array_t&& a)
      value_->type = value_type::array;
      return *this;
    }

    JsonValue& operator=(const object_t& o)
    {
      value_->~value_type();
      value_->object_ = new object_t{};
      for(auto& [k, v]: o)
      {
        value_->object_->emplace(k, v.clone());
      }
      value_->type = value_type::object;
      return *this;
    }

    JsonValue& operator=(object_t&& o)
    {
      value_->~value_type();
      value_->object_ = new object_t{std::move(o)};
      value_->type = value_type::object;
      return *this;
    }

    [[nodiscard]] const std::string& trace() const { return trace_; }

    static JsonValue from_trace(const std::string& t)
    {
      JsonValue res{};
      res.trace_ = t;
      return res;
    }

    [[nodiscard]] bool is_boolean() const { return value_->type == value_type::boolean; }

    [[nodiscard]] bool is_null() const { return value_->type == value_type::null; }

    [[nodiscard]] bool is_number() const { return value_->type == value_type::number; }

    [[nodiscard]] bool is_string() const { return value_->type == value_type::string; }

    [[nodiscard]] bool is_array() const { return value_->type == value_type::array; }

    [[nodiscard]] bool is_object() const { return value_->type == value_type::object; }

    explicit operator bool() const { return value_->type != value_type::invalid; }

    template<typename T>
    T* get()
    {
      auto type = detail::type_map<T>::type;
      if(value_->type != type || type == value_type::invalid)
      {
        return nullptr;
      }
      return detail::value_map<T>::value(*value_);
    }

    // no check
    template<typename T>
    T& as()
    {
      return *detail::value_map<T>::value(*value_);
    }

    JsonValue& operator[](size_t i) { return (*value_->array_)[i]; }

    JsonValue& operator[](const string_t& k) { return (*value_->object_)[k]; }

    [[deprecated("use to_string2() instead")]]
    std::string to_string(bool* ok = nullptr) const { return to_string(true, 0, ok); }

    [[deprecated("use to_string2() instead")]]
    std::string to_string(size_t indent, bool* ok = nullptr) const { return to_string(false, indent, ok); }

    std::string to_string2(size_t indent = 0) const
    {
      if(!*this)
      {
        return "invalid JsonValue";
      }
      enum Status
      {
        Array = 0,
        Key,
        Value
      };
      std::stack<value_ptr> st{};
      std::stack<Status> ct{};
      std::stack<value_ptr> tr{};
      size_t cur_indent = indent;
      bool compact = indent == 0;
      st.push(value_);
      std::string os;

      while(!st.empty())
      {
        auto j = st.top();
        st.pop();

        switch(j->type)
        {
        case detail::value_type::null:
        {
          if(!ct.empty())
          {
            switch(ct.top())
            {
            case Array:
              make_space(cur_indent, os, compact);
              [[fallthrough]];
            case Value:
              os.append("null");
              os.push_back(',');
              break;
            default:
              break;
            }
            new_line(os, compact);
            ct.pop();
          }
          else
          {
            os.append("null");
          }
          break;
        }
        case detail::value_type::boolean:
        {
          if(!ct.empty())
          {
            switch(ct.top())
            {
            case Array:
              make_space(cur_indent, os, compact);
              [[fallthrough]];
            case Value:
              os.append(j->bool_ ? "true" : "false");
              os.push_back(',');
              break;
            default:
              break;
            }
            new_line(os, compact);
            ct.pop();
          }
          else
          {
            os.append(j->bool_ ? "true" : "false");
          }
          break;
        }
        case detail::value_type::number:
        {
          char buf[64]{};
          int n = snprintf(buf, sizeof(buf), "%f", j->number_);
          assert(n > 0);
          std::string s{buf, (size_t)n};
          s = s.substr(0, s.find_last_not_of('0') + 1);
          if(!s.empty() && s.back() == '.')
          {
            s.pop_back();
          }

          if(!ct.empty())
          {
            switch(ct.top())
            {
            case Array:
              make_space(cur_indent, os, compact);
              [[fallthrough]];
            case Value:
              os.append(s);
              os.push_back(',');
              break;
            default:
              break;
            }
            new_line(os, compact);
            ct.pop();
          }
          else
          {
            os.append(s);
          }
          break;
        }
        case detail::value_type::string:
        {
          if(!ct.empty())
          {
            switch(ct.top())
            {
            case Array:
              make_space(cur_indent, os, compact);
              escape_str(os, *j->string_);
              os.push_back(',');
              new_line(os, compact);
              break;
            case Key:
              make_space(cur_indent, os, compact);
              escape_str(os, *j->string_);
              os.push_back(':');
              make_space(1, os, compact);
              break;
            case Value:
              escape_str(os, *j->string_);
              os.push_back(',');
              new_line(os, compact);
              break;
            }
            ct.pop();
          }
          else
          {
            escape_str(os, *j->string_);
          }
          break;
        }
        case detail::value_type::array:
        {
          if(tr.empty())
          {
            os.push_back('[');
            cur_indent += indent;
            tr.push(j); // trace
            st.push(j); // back to here
            for(auto& v: *j->array_)
            {
              ct.push(Array);
              st.push(v.value_);
            }
          }
          else if(tr.top() == j)
          {
            cur_indent -= indent;
            if(j->array_->empty())
            {
              os.push_back(']');
            }
            else
            {
              if(!ct.empty())
              {
                ct.pop(); // remove previous unpacked object
              }
              os.erase(os.find_last_of(',')); // os always end at ','
              new_line(os, compact);
              make_space(cur_indent, os, compact);
              os.push_back(']');
              os.push_back(',');
            }
            tr.pop();
          }
          else
          {
            if(!ct.empty() && ct.top() != Value)
            {
              make_space(cur_indent, os, compact);
            }
            cur_indent += indent;
            os.push_back('[');
            tr.push(j);
            st.push(j);
            for(auto& v: *j->array_)
            {
              ct.push(Array);
              st.push(v.value_);
            }
          }
          new_line(os, compact);
          break;
        }
        case detail::value_type::object:
        {
          if(tr.empty())
          {
            os.push_back('{');
            tr.push(j);
            st.push(j); // back to here
            for(auto& [k, v]: *j->object_)
            {
              // invert
              ct.push(Value);
              st.push(v.value_);

              ct.push(Key); // dirty and quick
              auto key = std::make_shared<value_type>();
              key->string_ = new string_t{k};
              key->type = detail::value_type::string;
              st.push(key);
            }
          }
          else if(tr.top() == j)
          {
            cur_indent -= indent;
            if(j->object_->empty())
            {
              // make_space(cur_indent, os, compact);
              os.push_back('}');
            }
            else
            {
              if(!ct.empty())
              {
                ct.pop(); // remove previous unpacked object
              }
              os.erase(os.find_last_of(',')); // os always end at ','
              new_line(os, compact);
              make_space(cur_indent, os, compact);
              os.push_back('}');
              os.push_back(',');
            }
            tr.pop();
          }
          else
          {
            if(!ct.empty() && ct.top() != Value)
            {
              make_space(cur_indent, os, compact);
            }
            cur_indent += indent;
            os.push_back('{');
            tr.push(j);
            st.push(j);
            for(auto& [k, v]: *j->object_)
            {
              // invert
              ct.push(Value);
              st.push(v.value_);

              ct.push(Key);
              auto key = std::make_shared<value_type>();
              key->string_ = new string_t{k};
              key->type = detail::value_type::string;
              st.push(key);
            }
          }
          new_line(os, compact);
          break;
        }
        default:
          return "invalid JsonValue";
        }
      }
      if(auto n = os.find_last_of(','); n != std::string::npos)
      {
        os.erase(n);
      }
      return os;
    }

    // deep copy
    JsonValue clone() const
    {
      JsonValue res{};
      clone(*this, res);
      return res;
    }

    bool operator==(const JsonValue& r) const
    {
      std::stack<JsonValue> sl;
      std::stack<JsonValue> sr;

      sl.push(*this);
      sr.push(r);

      while(!sl.empty() && !sr.empty())
      {
        auto nl = sl.top();
        sl.pop();
        auto nr = sr.top();
        sr.pop();

        if(nl.value_->type != nr.value_->type)
        {
          return false;
        }
        switch(nl.value_->type)
        {
        case detail::value_type::boolean:
        {
          if(nl.value_->bool_ != nr.value_->bool_)
          {
            return false;
          }
          break;
        }
        case detail::value_type::number:
        {
          if(std::abs(nl.value_->number_ - nr.value_->number_) > std::numeric_limits<number_t>::epsilon())
          {
            return false;
          }
          break;
        }
        case detail::value_type::string:
        {
          if(*nl.value_->string_ != *nr.value_->string_)
          {
            return false;
          }
          break;
        }
        case detail::value_type::array:
        {
          if(nl.value_->array_->size() != nr.value_->array_->size())
          {
            return false;
          }
          for(auto& x: *nl.value_->array_)
          {
            sl.push(x);
          }
          for(auto& x: *nr.value_->array_)
          {
            sr.push(x);
          }
          break;
        }
        case detail::value_type::object:
        {
          if(nl.value_->object_->size() != nr.value_->object_->size())
          {
            return false;
          }
          auto si = nl.value_->object_->begin();
          auto ri = nr.value_->object_->begin();
          auto e = nl.value_->object_->end();
          while(si != e)
          {
            if(si->first != ri->first)
            {
              return false;
            }
            sl.push(si->second);
            sr.push(ri->second);
            ++si;
            ++ri;
          }
          break;
        }
        default:
          break;
        }
      }
      return sr.size() == sl.size();
    }

  private:
    using value_ptr = std::shared_ptr<value_type>;
    value_ptr value_;
    std::string trace_;

    std::string to_string(bool compact, size_t indent, bool* ok) const
    {
      if(!(bool)*this)
      {
        if(ok)
        {
          *ok = false;
        }
        return "invalid JsonValue";
      }
      size_t depth = 0;
      std::string os;
      to_string_impl(os, indent, indent, true, depth, compact);
      if(depth > max_depth)
      {
        if(ok)
        {
          *ok = false;
        }
        return "nested too deeply, nest depth is limited to " + std::to_string(max_depth);
      }
      return os;
    }

    void to_string_impl(std::string& os, size_t indent, size_t cur_indent, bool top, size_t& depth, bool compact) const
    {
      switch(value_->type)
      {
      case value_type::null:
        os.append("null");
        break;
      case value_type::invalid:
        break;
      case value_type::boolean:
        os.append(value_->bool_ ? "true" : "false");
        break;
      case value_type::number:
      {
        char buf[64]{};
        int n = snprintf(buf, sizeof(buf), "%f", value_->number_);
        assert(n > 0);
        std::string s{buf, (size_t)n};
        s = s.substr(0, s.find_last_not_of('0') + 1);
        if(!s.empty() && s.back() == '.')
        {
          s.pop_back();
        }
        os.append(s);
        break;
      }
      case value_type::array:
      {
        if(depth > max_depth)
        {
          return;
        }
        ++depth;
        os.push_back('[');
        new_line(os, compact);
        size_t n = value_->array_->size();
        size_t idx = 0;
        for(auto& v: *value_->array_)
        {
          make_space(cur_indent, os, compact);
          v.to_string_impl(os, indent, cur_indent + indent, false, depth, compact);
          if(idx < n - 1)
          {
            os.push_back(',');
          }
          new_line(os, compact);
          idx += 1;
        }
        if(!top)
        {
          make_space(cur_indent - indent, os, compact);
        }
        os.push_back(']');
        --depth;
        break;
      }
      case value_type::string:
      {
        escape_str(os, *value_->string_);
        break;
      }
      case value_type::object:
      {
        if(depth > max_depth)
        {
          return;
        }
        ++depth;
        size_t idx = 0;
        size_t size = value_->object_->size();
        os.push_back('{');
        new_line(os, compact);
        for(auto& [k, v]: *value_->object_)
        {
          make_space(cur_indent, os, compact);
          escape_str(os, k);
          os.push_back(':');
          make_space(1, os, compact);
          v.to_string_impl(os, indent, cur_indent + indent, false, depth, compact);
          if(idx < size - 1)
          {
            os.push_back(',');
          }
          new_line(os, compact);
          idx += 1;
        }
        if(!top)
        {
          make_space(cur_indent - indent, os, compact);
        }
        os.push_back('}');
        --depth;
        break;
      }
      }
    }

    static void clone(const JsonValue& in, JsonValue& out)
    {
      std::stack<JsonValue> si{};
      si.push(in);
      std::stack<JsonValue> so{};
      so.push(out);
      while(!si.empty())
      {
        auto ji = si.top();
        si.pop();
        auto jo = so.top();
        so.pop();

        auto& vi = ji.value_;
        auto& vo = jo.value_;

        switch(vi->type)
        {
        case detail::value_type::invalid:
          return;
        case detail::value_type::null:
        {
          vo->type = vi->type;
          vo->null_ = null_t{};
          break;
        }
        case detail::value_type::boolean:
        {
          vo->type = vi->type;
          vo->bool_ = vi->bool_;
          break;
        }
        case detail::value_type::number:
        {
          vo->type = vi->type;
          vo->number_ = vi->number_;
          break;
        }
        case detail::value_type::string:
        {
          vo->type = vi->type;
          vo->string_ = new std::string{*vi->string_};
          break;
        }
        case detail::value_type::array:
        {
          vo->type = vi->type;
          vo->array_ = new array_t{};
          auto n = vi->array_->size();
          vo->array_->resize(n);
          for(size_t i = 0; i < n; ++i)
          {
            si.push((*vi->array_)[i]);
            so.push((*vo->array_)[i]);
          }
          break;
        }
        case detail::value_type::object:
        {
          vo->type = vi->type;
          vo->object_ = new object_t{};
          for(auto& [k, v]: *vi->object_)
          {
            si.push(v);
            so.push((*vo->object_)[k]);
          }
          break;
        }
        }
      }
    }

    static void make_space(size_t n, std::string& os, bool compact)
    {
      if(!compact)
      {
        while(n-- > 0)
        {
          os.push_back(' ');
        }
      }
    }

    static void new_line(std::string& os, bool compact)
    {
      if(!compact)
      {
        os.push_back('\n');
      }
    }

    static void escape_str(std::string& os, const std::string& s)
    {
      os.push_back('"');
      for(auto c: s)
      {
        if(c == '"' || c == '\\')
        {
          os.push_back('\\');
        }
        os.push_back(c);
      }
      os.push_back('"');
    }
  };

  inline std::ostream& operator<<(std::ostream& os, const JsonValue& j)
  {
    os << j.to_string2();
    return os;
  }

  namespace detail
  {
    using namespace json;
    enum Token
    {
      Comma = 0,
      Colon,
      BracketOn,
      BracketOff,
      BraceOn,
      BraceOff,
      String,
      Number,
      BooleanTrue,
      BooleanFalse,
      Null,
      Invalid
    };

    class Tokenizer
    {
    public:
      explicit Tokenizer(const std::string& src) : idx_next_{0}, line_{0}, src_{src} {}
      ~Tokenizer() = default;

      nullable<Token> next()
      {
        while(true)
        {
          auto iter = next_ch();
          if(!iter)
          {
            break;
          }
          switch(*iter)
          {
          case ',':
          {
            return Token::Comma;
          }
          case ':':
            return Token::Colon;
          case '[':
            return Token::BracketOn;
          case ']':
            return Token::BracketOff;
          case '{':
            return Token::BraceOn;
          case '}':
            return Token::BraceOff;
          case '-':
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            return Token::Number;
          case '"':
            return Token::String;
          case 'a':
          case 'e':
          case 'f':
          case 'l':
          case 'n':
          case 'r':
          case 's':
          case 't':
          case 'u':
          {
            auto sym = this->read_symbol(*iter);
            if(!sym)
            {
              return Token::Invalid;
            }
            if(*sym == "true")
            {
              return Token::BooleanTrue;
            }
            else if(*sym == "false")
            {
              return Token::BooleanFalse;
            }
            else if(*sym == "null")
            {
              return Token::Null;
            }
            else
            {
              return Token::Invalid;
            }
          }
          default:
          {
            if(::isspace(*iter))
            {
              if(*iter == '\n')
              {
                line_ += 1;
              }
              continue;
            }
            return Token::Invalid;
          }
          }
        }
        return {};
      }

      nullable<Token> peek()
      {
        auto saved = idx_next_;
        auto r = next();
        idx_next_ = saved;
        return r;
      }

      // ensure call after next or behavior is undefined
      char ch() { return src_[idx_next_ - 1]; }

      std::string trace()
      {
        if(idx_next_ == 0)
        {
          return {};
        }
        std::ostringstream os;
        os << "line: " << line_ << ", offset: " << (idx_next_ - 1)
           << ", invalid token around: " << src_.substr(idx_next_ - 1, 10);
        return os.str();
      }

      nullable<std::string> read_string(char first)
      {
        std::string res;
        bool escape = false;
        auto saved_idx = idx_next_ - 1; // simple trace
        while(true)
        {
          auto ch = this->next_ch();
          if(!ch)
          {
            idx_next_ = saved_idx;
            return {};
          }
          if(!escape && *ch == first) // next_ch will eat '"'
          {
            break;
          }
          switch(*ch)
          {
          case '\\':
            if(escape)
            {
              escape = false;
              res.push_back(*ch);
            }
            else
            {
              escape = true;
            }
            break;
          default:
            res.push_back(*ch);
            escape = false;
          }
        }
        return res;
      }

      // not including NaN, -Inf, Inf
      nullable<double> read_number(char ch)
      {
        std::string tmp;
        auto saved_idx = idx_next_ - 1;
        tmp.push_back(ch);
        bool doton = false;
        while(true)
        {
          auto n = this->next_ch(); // never eat none-is_number character
          if(!n)
          {
            idx_next_ = saved_idx;
            return {};
          }
          if((*n >= '0' && *n <= '9'))
          {
            tmp.push_back(*n);
          }
          else if(!doton && *n == '.')
          {
            doton = true;
            tmp.push_back(*n);
          }
          else
          {
            this->back();
            break;
          }
        }
        try
        {
          return std::stod(tmp);
        }
        catch(...)
        {
          return {};
        }
      }

      nullable<std::string> read_symbol(char first)
      {
        std::string res;
        res.push_back(first);
        auto saved_idx = idx_next_ - 1;
        while(true)
        {
          auto ch = this->next_ch(); // never eat none-symbol character
          if(!ch)
          {
            idx_next_ = saved_idx;
            return {};
          }
          if(!isalpha(*ch))
          {
            this->back();
            break;
          }
          res.push_back(*ch);
        }
        return res;
      }

    private:
      size_t idx_next_;
      size_t line_;
      const std::string& src_;

      nullable<char> next_ch()
      {
        if(idx_next_ == src_.size())
        {
          return {};
        }
        char c = src_[idx_next_++];
        return c;
      }

      void back()
      {
        if(idx_next_ > 0)
        {
          idx_next_ -= 1;
        }
      }
    };

    class Parser
    {
    public:
      explicit Parser(const std::string& src) : depth_{0}, token_{src} {}

      JsonValue parse()
      {
        auto token = token_.next();
        if(!token || *token == Token::Invalid)
        {
          return JsonValue::from_trace(token_.trace());
        }
        return this->parse_next(*token);
      }

      JsonValue parse2()
      {
        auto token = token_.next();
        if(!token)
        {
          return {};
        }
        std::stack<JsonValue> st{};
        st.push({});

        std::stack<Token> pairs{};
        std::stack<JsonValue> tmp;
        bool is_key = true;

        while(!st.empty())
        {
          auto& j = st.top();
          switch(*token)
          {
          case Invalid:
          {
            return JsonValue::from_trace(token_.trace());
          }
          case Null:
          case BooleanTrue:
          case BooleanFalse:
          case Number:
          {
            number_t num = 0;
            if(*token == Number)
            {
              auto n = token_.read_number(token_.ch());
              if(!n)
              {
                return JsonValue::from_trace(token_.trace());
              }
              num = *n;
            }

            auto next = token_.peek();
            if(!next)
            {
              if(!pairs.empty() && (pairs.top() == BraceOn || pairs.top() == BracketOn)) // need more
              {
                return JsonValue::from_trace(token_.trace());
              }
            }

            if(pairs.empty())
            {
              break;
            }
            if(pairs.top() == BracketOn)
            {
              switch(*next)
              {
              case Comma:
              case BracketOff:
                break;
              default:
                return JsonValue::from_trace(token_.trace());
              }
            }
            else if(pairs.top() == BraceOn)
            {
              switch(*next)
              {
              case Comma:
              case BraceOff:
                break;
              default:
                return JsonValue::from_trace(token_.trace());
              }
            }

            switch(*token)
            {
            case Null:
              j = null_t{};
              break;
            case BooleanFalse:
            case BooleanTrue:
              j = *token == BooleanTrue;
              break;
            case Number:
              j = num;
              break;
            default:
              break;
            }
            break;
          }
          case String:
          {
            auto s = token_.read_string(token_.ch());
            if(!s)
            {
              return JsonValue::from_trace(token_.trace());
            }
            auto next = token_.peek();
            if(!next)
            {
              if(!pairs.empty() && (pairs.top() == BraceOn || pairs.top() == BracketOn))
              {
                return JsonValue::from_trace(token_.trace());
              }
            }
            j = std::move(*s); // dirty and quick, no need to distinguish the key or value for object
            if(pairs.empty())
            {
              break;
            }
            if(pairs.top() == BracketOn)
            {
              switch(*next)
              {
              case Comma:
              case BracketOff:
                break;
              default:
                return JsonValue::from_trace(token_.trace());
              }
            }
            else if(pairs.top() == BraceOn)
            {
              if(is_key)
              {
                if(*next != Colon)
                {
                  return JsonValue::from_trace(token_.trace());
                }
              }
              else
              {
                switch(*next)
                {
                case Comma:
                case BraceOff:
                  break;
                default:
                  return JsonValue::from_trace(token_.trace());
                }
              }
            }
            break;
          }
          case BracketOn:
          {
            auto next = token_.peek();
            if(!next)
            {
              return JsonValue::from_trace(token_.trace());
            }
            switch(*next)
            {
            case Comma:
            case Colon:
            case BraceOff:
              return JsonValue::from_trace(token_.trace());
            default:
              break;
            }
            j = array_t{};
            pairs.push(*token);
            break;
          }
          case Comma:
          {
            if(pairs.empty())
            {
              return JsonValue::from_trace(token_.trace());
            }
            auto next = token_.peek();
            if(!next)
            {
              return JsonValue::from_trace(token_.trace());
            }
            if(pairs.top() == BracketOn)
            {
              switch(*next)
              {
              case Invalid:
              case Comma:
              case Colon:
              case BracketOff:
              case BraceOff:
                return JsonValue::from_trace(token_.trace());
              default:
                break;
              }
            }
            else if(pairs.top() == BraceOn)
            {
              switch(*next)
              {
              case String:
              case BraceOff:
                break;
              default:
                return JsonValue::from_trace(token_.trace());
              }
              is_key = true;
            }
            break;
          }
          case Colon:
          {
            if(pairs.empty() || pairs.top() != BraceOn)
            {
              return JsonValue::from_trace(token_.trace());
            }
            auto next = token_.peek();
            if(!next)
            {
              return JsonValue::from_trace(token_.trace());
            }
            switch(*next)
            {
            case Invalid:
            case Comma:
            case Colon:
            case BracketOff:
            case BraceOff:
              return JsonValue::from_trace(token_.trace());
            default:
              break;
            }
            is_key = false;
            break;
          }
          case BracketOff:
          {
            auto next = token_.peek();
            if(next)
            {
              switch(*next)
              {
              case Comma:
              case BracketOff:
              case BraceOff:
                break;
              default:
                return JsonValue::from_trace(token_.trace());
              }
            }
            if(pairs.empty() || pairs.top() != BracketOn)
            {
              return JsonValue::from_trace(token_.trace());
            }

            while(!st.empty())
            {
              if(st.top().is_array() && st.top().as<array_t>().empty())
              {
                break;
              }
              tmp.push(st.top());
              st.pop();
            }
            if(st.empty())
            {
              return JsonValue::from_trace(token_.trace());
            }
            auto& parent = st.top();
            while(!tmp.empty())
            {
              parent.as<array_t>().push_back(std::move(tmp.top()));
              tmp.pop();
            }
            pairs.pop();
            break;
          }
          case BraceOn:
          {
            auto next = token_.peek();
            if(!next)
            {
              return JsonValue::from_trace(token_.trace());
            }
            switch(*next)
            {
            case String:
            case BraceOff:
              break;
            default:
              return JsonValue::from_trace(token_.trace());
            }
            j = object_t{};
            pairs.push(*token);
            is_key = true;
            break;
          }
          case BraceOff:
          {
            auto next = token_.peek();
            if(next)
            {
              switch(*next)
              {
              case Comma:
              case BracketOff:
              case BraceOff:
                break;
              default:
                return JsonValue::from_trace(token_.trace());
              }
            }
            if(pairs.empty() || pairs.top() != BraceOn)
            {
              return JsonValue::from_trace(token_.trace());
            }
            while(!st.empty())
            {
              if(st.top().is_object() && st.top().as<object_t>().empty())
              {
                break;
              }
              tmp.push(st.top());
              st.pop();
            }
            if(st.empty() || (tmp.size() % 2 != 0)) // not pair
            {
              return JsonValue::from_trace(token_.trace());
            }
            auto& parent = st.top();
            while(!tmp.empty())
            {
              auto k = tmp.top();
              tmp.pop();
              auto v = tmp.top();
              tmp.pop();
              parent.as<object_t>().emplace(k.as<string_t>(), std::move(v));
            }
            pairs.pop();
            break;
          }
          }

          token = token_.next();
          if(!token)
          {
            if(!pairs.empty())
            {
              break;
            }
            return st.top();
          }
          if(st.top() && *token != BraceOff && *token != BracketOff) // skip invalid, comma and colon
          {
            st.push({});
          }
        }
        return JsonValue::from_trace(token_.trace());
      }

    private:
      size_t depth_;
      Tokenizer token_;

      JsonValue parse_next(Token token)
      {
        if(depth_ > max_depth)
        {
          return JsonValue::from_trace("nested too deeply, nest depth is limited to " + std::to_string(max_depth));
        }
        switch(token)
        {
        case Token::BooleanTrue:
        {
          return true;
        }
        case Token::BooleanFalse:
        {
          return false;
        }
        case Token::Number:
        {
          auto num = token_.read_number(token_.ch());
          if(!num)
          {
            return JsonValue::from_trace(token_.trace());
          }
          return *num;
        }
        case Token::String:
        {
          auto str = token_.read_string('"');
          if(!str)
          {
            return JsonValue::from_trace(token_.trace());
          }
          return std::move(*str);
        }
        case Token::BracketOn:
        {
          return this->parse_array();
        }
        case Token::BraceOn:
        {
          return this->parse_object();
        }
        case Token::Null:
        {
          return null_t{};
        }
        default:
        {
          return JsonValue::from_trace(token_.trace());
        }
        }
      }

      JsonValue parse_array()
      {
        ++depth_;
        auto token = token_.next();
        if(!token || token == Token::Invalid)
        {
          return JsonValue::from_trace(token_.trace());
        }

        array_t a;
        if(token == Token::BracketOff)
        {
          --depth_;
          return a;
        }
        auto v = this->parse_next(*token);
        if(!v) // invalid element
        {
          return v;
        }
        a.push_back(std::move(v));

        bool done = false;
        while(!done)
        {
          token = token_.next();
          switch(*token)
          {
          case Token::Comma:
          {
            v = this->parse();
            if(!v)
            {
              return v;
            }
            a.push_back(std::move(v));
            break;
          }
          case Token::BracketOff:
          {
            --depth_;
            done = true;
            break;
          }
          default:
          {
            return JsonValue::from_trace(token_.trace());
          }
          }
        }
        return a;
      }

      JsonValue parse_object()
      {
        ++depth_;
        auto token = token_.next();
        if(!token || *token == Token::Invalid)
        {
          return JsonValue::from_trace(token_.trace());
        }
        object_t o;

        // parse one or zero k-v pair
        switch(*token)
        {
        case Token::BraceOff:
        {
          --depth_;
          return o;
        }
        case Token::String:
        {
          auto key = token_.read_string('"');
          if(!key)
          {
            return JsonValue::from_trace(token_.trace());
          }
          auto token2 = token_.next();
          switch(*token2)
          {
          case Token::Colon:
            break;
          default:
            return JsonValue::from_trace(token_.trace());
          }
          auto value = this->parse(); // maybe sub-object
          if(!value) // invalid value
          {
            return value;
          }
          o.emplace(*key, std::move(value));
          break;
        }
        default:
        {
          return JsonValue::from_trace(token_.trace());
        }
        }

        // parse rest k-v pairs
        bool done = false;
        while(!done)
        {
          token = token_.next();
          switch(*token)
          {
          case Token::Comma:
          {
            nullable<std::string> key;
            auto token2 = token_.next();
            if(!token2)
            {
              return JsonValue::from_trace(token_.trace());
              ;
            }
            switch(*token2)
            {
            case Token::String:
              key = token_.read_string('"');
              if(!key)
              {
                return JsonValue::from_trace(token_.trace());
              }
              break;
            default:
              return JsonValue::from_trace(token_.trace());
            }
            token2 = token_.next();
            switch(*token2)
            {
            case Token::Colon:
              break;
            default:
              return JsonValue::from_trace(token_.trace());
            }
            auto value = this->parse();
            if(!value)
            {
              return value;
            }
            o.emplace(*key, std::move(value));
            break;
          }
          case Token::BraceOff:
            --depth_;
            done = true;
            break;
          default:
            return JsonValue::from_trace(token_.trace());
          }
        }
        return o;
      }
    };
  } // namespace detail

  [[deprecated("use parse2() instead")]]
  JsonValue parse(const std::string& src)
  {
    detail::Parser p{src};
    return p.parse();
  }

  // no stack limit
  // NOTE: the result of parse2() is inverted (while parse() is as its input)
  JsonValue parse2(const std::string& src)
  {
    detail::Parser p{src};
    return p.parse2();
  }
} // namespace nm::json

#endif // JSON_H_
