/***********************************************
        File Name: json.h
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 12/28/20 10:40 AM
***********************************************/

#ifndef JSON_H_
#define JSON_H_

#include <map>
#include <memory>
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

    struct value_type
    {
      value_type() : data{null_t{}}, type{invalid} {}

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
      union Data
      {
        null_t null_;
        bool_t bool_;
        number_t number_;
        array_t* array_;
        string_t* string_;
        object_t* object_;
      };

      Data data;
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
      static constexpr null_t* value(value_type::Data& v) { return &v.null_; }
    };
    template<>
    struct value_map<bool_t>
    {
      static constexpr bool_t* value(value_type::Data& v) { return &v.bool_; }
    };
    template<>
    struct value_map<number_t>
    {
      static constexpr number_t* value(value_type::Data& v) { return &v.number_; }
    };
    template<>
    struct value_map<array_t>
    {
      static constexpr array_t* value(value_type::Data& v) { return v.array_; }
    };
    template<>
    struct value_map<string_t>
    {
      static constexpr string_t* value(value_type::Data& v) { return v.string_; }
    };
    template<>
    struct value_map<object_t>
    {
      static constexpr object_t* value(value_type::Data& v) { return v.object_; }
    };
  } // namespace detail

  class JsonValue final
  {
    using value_type = detail::value_type;
    static void deleter(value_type* v)
    {
      switch(v->type)
      {
      case value_type::string:
      {
        delete v->data.string_;
        break;
      }
      case value_type::object:
      {
        delete v->data.object_;
        break;
      }
      case value_type::array:
      {
        delete v->data.array_;
        break;
      }
      default:
        break;
      }
      delete v;
    }

  public:
    JsonValue() : value_{new value_type{}, deleter}, trace_{} {}
    template<size_t N>
    JsonValue(const char (&a)[N]) : JsonValue{string_t{a, N}}
    {
    }
    // no explicit
    JsonValue(string_t&& s) : JsonValue{}
    {
      value_->data.string_ = new std::string{std::move(s)};
      value_->type = value_type::string;
    }
    JsonValue(number_t d) : JsonValue{}
    {
      value_->data.number_ = d;
      value_->type = value_type::number;
    }
    JsonValue(null_t n) : JsonValue{}
    {
      value_->data.null_ = n;
      value_->type = value_type::null;
    }
    JsonValue(bool_t b) : JsonValue{}
    {
      value_->data.bool_ = b;
      value_->type = value_type::boolean;
    }
    JsonValue(array_t&& a) : JsonValue{}
    {
#if 1
      value_->data.array_ = new array_t{};
      *value_->data.array_ = std::move(a);
#else
      // recursively call itself when using g++ (clang++ is ok)
      value_->data.array_ = new array_t{std::move(a)};
#endif
      value_->type = value_type::array;
    }
    JsonValue(object_t&& o) : JsonValue{}
    {
      value_->data.object_ = new object_t{std::move(o)};
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
    T* as()
    {
      auto type = detail::type_map<T>::type;
      if(value_->type != type || type == value_type::invalid)
      {
        return nullptr;
      }
      return detail::value_map<T>::value(value_->data);
    }

    // no check
    template<typename T>
    T& get()
    {
      return *detail::value_map<T>::value(value_->data);
    }

    std::string to_string(bool* ok = nullptr) const { return to_string(true, 0, ok); }

    std::string to_string(size_t indent, bool* ok = nullptr) const { return to_string(false, indent, ok); }

  private:
    std::shared_ptr<value_type> value_;
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
      auto r = to_string_impl(indent, indent, true, depth, compact);
      if(depth > max_depth)
      {
        if(ok)
        {
          *ok = false;
        }
        return "nested too deeply, nest depth is limited to " + std::to_string(max_depth);
      }
      return r;
    }

    std::string to_string_impl(size_t indent, size_t cur_indent, bool top, size_t& depth, bool compact) const
    {
      switch(value_->type)
      {
      case value_type::null:
        return "null";
      case value_type::invalid:
        return "";
      case value_type::boolean:
        return value_->data.bool_ ? "true" : "false";
      case value_type::number:
        return std::to_string(value_->data.number_);
      case value_type::array:
      {
        if(depth > max_depth)
        {
          return {};
        }
        ++depth;
        std::ostringstream os;
        os << '[';
        new_line(os, compact);
        size_t n = value_->data.array_->size();
        size_t idx = 0;
        for(auto& v: *value_->data.array_)
        {
          make_space(cur_indent, os, compact);
          os << v.to_string_impl(indent, cur_indent + indent, false, depth, compact);
          if(idx < n - 1)
          {
            os << ',';
          }
          new_line(os, compact);
          idx += 1;
        }
        if(!top)
        {
          make_space(cur_indent - indent, os, compact);
        }
        os << ']';
        --depth;
        return os.str();
      }
      case value_type::string:
        return "\"" + *value_->data.string_ + "\"";
      case value_type::object:
      {
        if(depth > max_depth)
        {
          return {};
        }
        ++depth;
        std::ostringstream os;
        size_t idx = 0;
        size_t size = value_->data.object_->size();
        os << '{';
        new_line(os, compact);
        for(auto& [k, v]: *value_->data.object_)
        {
          make_space(cur_indent, os, compact);
          os << '"' << k << "\":";
          if(!compact)
          {
            os << ' ';
          }
          os << v.to_string_impl(indent, cur_indent + indent, false, depth, compact);
          if(idx < size - 1)
          {
            os << ',';
          }
          new_line(os, compact);
          idx += 1;
        }
        if(!top)
        {
          make_space(cur_indent - indent, os, compact);
        }
        os << '}';
        --depth;
        return os.str();
      }
      }
      return {}; // shut up compiler
    }

    static void make_space(size_t n, std::ostringstream& os, bool compact)
    {
      if(!compact)
      {
        while(n-- > 0)
        {
          os << ' ';
        }
      }
    }

    static void new_line(std::ostringstream& os, bool compact)
    {
      if(!compact)
      {
        os << '\n';
      }
    }
  };

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

  JsonValue parse(const std::string& src)
  {
    detail::Parser p{src};
    return p.parse();
  }
} // namespace nm::json

#endif // JSON_H_
