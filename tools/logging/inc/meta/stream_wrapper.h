/*********************************************************
          File Name:stream_wrapper.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 04:03:42 PM CST
**********************************************************/

#ifndef LOG_STREAM_H_
#define LOG_STREAM_H_

#include "file.h"
#include "util.h"
#include "logbase.h"
#include <cstring>
#include <limits>
#include <string>

namespace nm
{
  namespace meta
  {
    template<unsigned int N>
    class Buffer : uniq
    {
      public:
        Buffer()
        {
          memset(buf_, 0, sizeof(char) * N);
          cur_ = buf_;
          end_ = buf_ + N;
        }
        ~Buffer() { }
        void append(const char* line, const size_t len)
        {
          if(rest() > len)
          {
            memcpy(cur_, line, len);
            cur_ += len;
          }
        }
        size_t rest() const { return end_ - cur_; }
        size_t size() const { return cur_ - buf_; }
        bool avail() const { return end_ != cur_; }
        const char* data() { return buf_; }
        void reset()
        {
          cur_ = buf_;
        }
        void clear()
        {
          memset(buf_, 0, sizeof(char) * N);
          cur_ = buf_;
        }
        std::string to_string() const
        {
          return std::string(buf_, cur_ - buf_);
        }
        const char* data() const
        {
          return buf_;
        }
      private:
        char buf_[N];
        char* cur_;
        char* end_;
    };
    class BufferStream : uniq
    {
      public:
        BufferStream(): buffer_(){}
        ~BufferStream() {}
        template<int N>
        BufferStream& operator<< (const char (&arr)[N])
        {
          buffer_.append(arr, N - 1);
          return *this;
        }
        template<typename T>
        BufferStream& operator<< (const T x)
        {
          meta::append(&buffer_, x);
          return *this;
        }
        BufferStream& operator<< (const std::string&);
        const char* buffer() const
        {
          return buffer_.data();
        }
        void append(const char* s, const size_t len);
        size_t size() const;
        void reset();
        void flush();
      private:
        enum
        {
          BUFFER_SIZE = 1024 * 4 // 4KB
        };
        Buffer<BUFFER_SIZE> buffer_;
    };

    class FileStream : uniq
    {
      public:
        FileStream(): file_(nullptr){}
        FileStream(meta::FileCtl* file) : file_(file){}
        ~FileStream(){ delete file_; }
        template<int N>
        FileStream& operator<< (const char (&arr)[N])
        {
          file_->append(arr, N - 1);
          return *this;
        }
        template<typename T>
        FileStream& operator<< (const T x)
        {
          meta::append(file_, x);
          return *this;
        }
        FileStream& operator<< (const std::string&);
        void set(FileCtl*);
        bool valid();
        void flush();
        void append(const char* s, size_t len);
      private:
        FileCtl* file_;
    };
  }
}

#endif
