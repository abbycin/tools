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
  template<unsigned int N>
  class Buffer
  {
    public:
      Buffer()
      {
        memset(buf_, 0, sizeof(char) * N);
        cur_ = buf_;
        end_ = buf_ + N;
      }
      Buffer(const Buffer&) = delete;
      Buffer(Buffer&&) = delete;
      Buffer& operator= (const Buffer&) = delete;
      Buffer& operator= (Buffer&&) = delete;
      ~Buffer() { }
      void append(const char* line, int len)
      {
        if(rest() > len)
        {
          memcpy(cur_, line, len);
          cur_ += len;
        }
      }
      int rest() const { return end_ - cur_; }
      size_t size() const { return cur_ - buf_; }
      bool avail() const { return !(end_ == cur_); }
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
        misc::append(&buffer_, x);
        return *this;
      }
      BufferStream& operator<< (const std::string&);
      const char* buffer() const
      {
        return buffer_.data();
      }
      void append(const char* s, int len)
      {
        buffer_.append(s, len);
      }
      size_t size() const
      {
        return buffer_.size();
      }
      void reset()
      {
        buffer_.reset();
      }
      void clear()
      {
        buffer_.clear();
      }
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
      FileStream(FileCtl* file) : file_(file){}
      void set(FileCtl* f) { file_ = f; }
      bool valid() { return file_ != nullptr; }
      void flush() { file_->flush(); }
      void append(const char* s, int len)
      {
        file_->append(s, len);
      }
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
        misc::append(file_, x);
        return *this;
      }
      FileStream& operator<< (const std::string&);
    private:
      FileCtl* file_;
  };
}

#endif
