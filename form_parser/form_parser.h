/***********************************************
        File Name: form_parser.h
        Author: Abby Cin
        Mail: abbytsing@gmail.com
        Created Time: 8/25/18 5:23 PM
***********************************************/

#ifndef FORM_DATA_PARSER_
#define FORM_DATA_PARSER_

#include <string>
#include <cstring>
#include <functional>
#include <limits>
#include <map>
#include <fstream>
#include <random>

namespace nm
{
#undef FALL_THROUGH
#if __cplusplus >= 201703L
#define FALL_THROUGH [[fallthrough]];
#else
#define FALL_THROUGH
#endif
  class FormParser
  {
    using CallBack = std::function<void(const char* buffer, size_t start, size_t end)>;

  public:
    using map_t = std::map<std::string, std::string>;

    FormParser()
    {
      lookbehind_ = nullptr;
      reset_callbacks();
      this->reset();
    }

    ~FormParser() { delete[] lookbehind_; }

    void set_env(const std::string& boundary, const std::string& path)
    {
      this->reset();
      this->boundary_ = "\r\n--" + boundary;
      tmp_path_ = path;
      if(tmp_path_[tmp_path_.size() - 1] != '/')
      {
        tmp_path_.append(1, '/');
      }
      boundary_data_ = this->boundary_.c_str();
      boundary_size_ = this->boundary_.size();
      this->index_boundary();
      lookbehind_ = new char[boundary_size_ + 8];
      lookbehind_size_ = boundary_size_ + 8;
      state_ = START;
      error_reason_ = "No error.";
    }

    const map_t& file_map() { return files_; }

    const map_t text_map() { return texts_; }

    void consume(const char* buffer, size_t len)
    {
      if(state_ == PARSE_ERROR || len == 0)
      {
        return;
      }

      State state = this->state_;
      int flags = this->flags_;
      size_t prev_index = this->index_;
      size_t index = this->index_;
      size_t boundaryEnd = boundary_size_ - 1;
      size_t i;
      char c, cl;

      for(i = 0; i < len; i++)
      {
        c = buffer[i];
        switch(state)
        {
        case PARSE_ERROR:
          return;
        case START:
          index = 0;
          state = START_BOUNDARY;
          FALL_THROUGH
        case START_BOUNDARY:
          if(index == boundary_size_ - 2)
          {
            if(c != CR)
            {
              set_error("Malformed. Expected CR after boundary.");
              return;
            }
            index++;
            break;
          }
          else if(index - 1 == boundary_size_ - 2)
          {
            if(c != LF)
            {
              set_error("Malformed. Expected LF after boundary CR.");
              return;
            }
            index = 0;
            callback(on_part_begin_);
            state = HEADER_FIELD_START;
            break;
          }
          if(c != boundary_[index + 2])
          {
            set_error("Malformed. Found different boundary data than the given one.");
            return;
          }
          index++;
          break;
        case HEADER_FIELD_START:
          state = HEADER_FIELD;
          header_field_mark_ = i;
          index = 0;
          FALL_THROUGH
        case HEADER_FIELD:
          if(c == CR)
          {
            header_field_mark_ = UNMARKED;
            state = HEADERS_ALMOST_DONE;
            break;
          }

          index++;
          if(c == DASH)
          {
            break;
          }

          if(c == COLON)
          {
            if(index == 1)
            {
              // empty header field
              set_error("Malformed first header name character.");
              return;
            }
            data_callback(on_header_field_, header_field_mark_, buffer, i, len, true);
            state = HEADER_VALUE_START;
            break;
          }

          cl = lower(c);
          if(cl < 'a' || cl > 'z')
          {
            set_error("Malformed header name.");
            return;
          }
          break;
        case HEADER_VALUE_START:
          if(c == SPACE)
          {
            break;
          }

          header_value_mark_ = i;
          state = HEADER_VALUE;
          FALL_THROUGH
        case HEADER_VALUE:
          if(c == CR)
          {
            data_callback(on_header_value_, header_value_mark_, buffer, i, len, true, true);
            callback(on_header_end_);
            state = HEADER_VALUE_ALMOST_DONE;
          }
          break;
        case HEADER_VALUE_ALMOST_DONE:
          if(c != LF)
          {
            set_error("Malformed header value: LF expected after CR");
            return;
          }

          state = HEADER_FIELD_START;
          break;
        case HEADERS_ALMOST_DONE:
          if(c != LF)
          {
            set_error("Malformed header ending: LF expected after CR");
            return;
          }
          callback(on_headers_end_);
          state = PART_DATA_START;
          break;
        case PART_DATA_START:
          state = PART_DATA;
          part_data_mark_ = i;
          FALL_THROUGH
        case PART_DATA:
          prev_index = index;

          if(index == 0)
          {
            i += boundaryEnd;
            // boyer-moore derived algorithm to safely skip non-boundary data
            while(i < len && !is_in_boundary(buffer[i]))
            {
              i += boundary_size_;
            }
            i -= boundaryEnd;
            c = buffer[i];
          }

          if(index < boundary_size_)
          {
            if(boundary_[index] == c)
            {
              if(index == 0)
              {
                data_callback(on_part_data_, part_data_mark_, buffer, i, len, true);
              }
              index++;
            }
            else
            {
              index = 0;
            }
          }
          else if(index == boundary_size_)
          {
            index++;
            if(c == CR)
            {
              // CR = part boundary
              flags |= PART_BOUNDARY;
            }
            else if(c == DASH)
            {
              // DASH = end boundary
              flags |= LAST_BOUNDARY;
            }
            else
            {
              index = 0;
            }
          }
          else if(index - 1 == boundary_size_)
          {
            if(flags & PART_BOUNDARY)
            {
              index = 0;
              if(c == LF)
              {
                // unset the PART_BOUNDARY flag
                flags &= ~PART_BOUNDARY;
                callback(on_part_end_);
                callback(on_part_begin_);
                state = HEADER_FIELD_START;
                break;
              }
            }
            else if(flags & LAST_BOUNDARY)
            {
              if(c == DASH)
              {
                callback(on_part_end_);
                callback(on_end_);
                state = END;
              }
              else
              {
                index = 0;
              }
            }
            else
            {
              index = 0;
            }
          }
          else if(index - 2 == boundary_size_)
          {
            if(c == CR)
            {
              index++;
            }
            else
            {
              index = 0;
            }
          }
          else if(index - boundary_size_ == 3)
          {
            index = 0;
            if(c == LF)
            {
              callback(on_part_end_);
              callback(on_end_);
              state = END;
              break;
            }
          }
          if(index > 0)
          {
            // when matching a possible boundary, keep a lookbehind reference
            // in case it turns out to be a false lead
            // since index is unsigned, this can happen either index equal 0
            // or index far more larger than lookbehind_size_
            if(index - 1 >= lookbehind_size_)
            {
              set_error("Parser bug: index overflows lookbehind buffer. "
                        "Please send bug report with input file attached.");
              return;
            }
            lookbehind_[index - 1] = c;
          }
          else if(prev_index > 0)
          {
            // if our boundary turned out to be rubbish, the captured lookbehind
            // belongs to partData
            callback(on_part_data_, lookbehind_, 0, prev_index);
            prev_index = 0;
            part_data_mark_ = i;

            // reconsider the current character even so it interrupted the sequence
            // it could be the beginning of a new sequence
            i--;
          }
          break;
        case END:
          break;
        default:
          return;
        }
      }

      data_callback(on_header_field_, header_field_mark_, buffer, i, len, false);
      data_callback(on_header_value_, header_value_mark_, buffer, i, len, false);
      data_callback(on_part_data_, part_data_mark_, buffer, i, len, false);

      this->index_ = index;
      this->state_ = state;
      this->flags_ = flags;
    }

    bool done() const { return state_ == END; }

    bool is_error() const { return state_ == PARSE_ERROR; }

    const char* err_string() const { return error_reason_; }

  private:
    enum State
    {
      PARSE_ERROR,
      START,
      START_BOUNDARY,
      HEADER_FIELD_START,
      HEADER_FIELD,
      HEADER_VALUE_START,
      HEADER_VALUE,
      HEADER_VALUE_ALMOST_DONE,
      HEADERS_ALMOST_DONE,
      PART_DATA_START,
      PART_DATA,
      PART_END,
      END
    };

    enum Flags
    {
      PART_BOUNDARY = 1,
      LAST_BOUNDARY = 2
    };

    enum ContentType
    {
      TEXT = 0,
      FILE,
      UNKNOWN
    };

    static const char CR = '\r';
    static const char LF = '\n';
    static const char SPACE = ' ';
    static const char DASH = '-';
    static const char COLON = ':';
    static const size_t UNMARKED = std::numeric_limits<size_t>::max();
    std::string boundary_;
    std::string tmp_path_;
    std::map<std::string, std::string> files_; // filename and tmp file path
    std::map<std::string, std::string> texts_; // name and value
    ContentType type_;
    std::string key_; // either name or filename
    std::string value_; // text value of key
    std::ofstream of_; // tmp file stream
    const char* boundary_data_;
    size_t boundary_size_;
    bool boundary_index_[256];
    char* lookbehind_;
    size_t lookbehind_size_;
    State state_;
    int flags_;
    size_t index_;
    size_t header_field_mark_;
    size_t header_value_mark_;
    size_t part_data_mark_;
    const char* error_reason_;
    CallBack on_part_begin_;
    CallBack on_header_field_;
    CallBack on_header_value_;
    CallBack on_header_end_; // one header parsed
    CallBack on_headers_end_; // all header parsed
    CallBack on_part_data_;
    CallBack on_part_end_;
    CallBack on_end_;

    void reset()
    {
      delete[] lookbehind_;
      state_ = PARSE_ERROR;
      type_ = UNKNOWN;
      boundary_.clear();
      boundary_data_ = boundary_.c_str();
      boundary_size_ = 0;
      lookbehind_ = nullptr;
      lookbehind_size_ = 0;
      flags_ = 0;
      index_ = 0;
      header_field_mark_ = UNMARKED;
      header_value_mark_ = UNMARKED;
      part_data_mark_ = UNMARKED;
      error_reason_ = "Parser uninitialized.";
    }

    std::string gen_temp_name(const std::string& prefix)
    {
      std::string res;
      res.reserve(prefix.size() + 33);
      res.append(prefix);
      static const char seed[] = "abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      static const int n = sizeof(seed) - 1;
      static std::random_device rd;
      std::mt19937 rng;
      rng.seed(rd());
      std::uniform_int_distribution<std::mt19937::result_type> dist(1, 1000);
      for(int i = 0; i < 33; ++i)
      {
        res.append(1, seed[dist(rng) % n]);
      }
      return res;
    }

    // data call back
    void on_header_filed(const char*, size_t, size_t) {}

    // data call back
    void on_header_value(const char* buf, size_t beg, size_t end)
    {
      std::string tmp{buf + beg, end - beg};
      static std::string name = "name=\"";
      static std::string filename = "filename=\"";
      // only file upload can both appear name and filename
      // so, if filename appeared, is must be file upload
      size_t index = tmp.find(filename);
      if(index != tmp.npos)
      {
        type_ = FILE;
        index += filename.size();
      }
      else if((index = tmp.find(name)) != tmp.npos)
      {
        type_ = TEXT;
        index += name.size();
      }

      if(type_ == UNKNOWN)
      {
        // current header don't contain name of filename, maybe
        // appear in next header
        return;
      }
      if(key_.empty())
      {
        key_ = tmp.substr(index, tmp.size() - index - 1);
      }
    }

    void on_header_end(const char*, size_t, size_t) {}

    void on_headers_end(const char*, size_t, size_t)
    {
      switch(type_)
      {
      case UNKNOWN:
        this->set_error("no name and filename given in header");
        // invalid header, skip it silently
        return;
      case TEXT:
        value_.clear();
        break;
      case FILE:
      {
        value_ = tmp_path_ + this->gen_temp_name(key_);
        of_.open(value_, of_.binary);
        if(!of_.is_open())
        {
          this->set_error("can't create temp file");
        }
        break;
      }
      }
    }

    void on_part_begin(const char*, size_t, size_t) {}

    // data call back
    void on_part_data(const char* buf, size_t beg, size_t end)
    {
      if(this->is_error())
      {
        return;
      }
      switch(type_)
      {
      case TEXT:
        value_.append(buf + beg, end - beg);
        break;
      case FILE:
        of_.write(buf + beg, end - beg);
        break;
      default:
        // impossible
        break;
      }
    }

    void on_part_end(const char*, size_t, size_t)
    {
      if(this->is_error())
      {

        return;
      }
      switch(type_)
      {
      case TEXT:
        texts_.insert({key_, value_});
        texts_.insert({key_, value_});
        break;
      case FILE:
        files_.insert({key_, value_});
        of_.close();
        break;
      default:
        break; // impossible
      }
      key_.clear();
      value_.clear();
      type_ = UNKNOWN; // re-init for next header of part
    }

    void on_end(const char*, size_t, size_t) {}

    void reset_callbacks()
    {
      on_part_begin_ = [this](const char* buf, size_t beg, size_t end) { this->on_part_begin(buf, beg, end); };
      on_header_field_ = [this](const char* buf, size_t beg, size_t end) { this->on_header_filed(buf, beg, end); };
      on_header_value_ = [this](const char* buf, size_t beg, size_t end) { this->on_header_value(buf, beg, end); };
      on_header_end_ = [this](const char* buf, size_t beg, size_t end) { this->on_header_end(buf, beg, end); };
      on_headers_end_ = [this](const char* buf, size_t beg, size_t end) { this->on_headers_end(buf, beg, end); };
      on_part_data_ = [this](const char* buf, size_t beg, size_t end) { this->on_part_data(buf, beg, end); };
      on_part_end_ = [this](const char* buf, size_t beg, size_t end) { this->on_part_end(buf, beg, end); };
      on_end_ = [this](const char* buf, size_t beg, size_t end) { this->on_end(buf, beg, end); };
    }

    void index_boundary()
    {
      const char* current;
      const char* end = boundary_data_ + boundary_size_;

      memset(boundary_index_, 0, sizeof(boundary_index_));

      for(current = boundary_data_; current < end; current++)
      {
        boundary_index_[(unsigned char)*current] = true;
      }
    }

    void callback(CallBack& cb, const char* buffer = nullptr, size_t start = UNMARKED, size_t end = UNMARKED,
                  bool allow_empty = false)
    {
      if(start != UNMARKED && start == end && !allow_empty)
      {
        return;
      }
      if(cb)
      {
        cb(buffer, start, end);
      }
    }

    void data_callback(CallBack& cb, size_t& mark, const char* buf, size_t i, size_t buf_len, bool clear,
                       bool allow_empty = false)
    {
      if(mark == UNMARKED)
      {
        return;
      }

      if(!clear)
      {
        callback(cb, buf, mark, buf_len, allow_empty);
        mark = 0;
      }
      else
      {
        callback(cb, buf, mark, i, allow_empty);
        mark = UNMARKED;
      }
    }

    char lower(char c) const
    {
      return c | 0x20; // 'A' ~ 'Z' => 'a' ~ 'z'
    }

    inline bool is_in_boundary(char c) const { return boundary_index_[(unsigned char)c]; }

    void set_error(const char* message)
    {
      state_ = PARSE_ERROR;
      error_reason_ = message;
    }
  };
}

#undef FALL_THROUGH
#endif // FORM_DATA_PARSER_
