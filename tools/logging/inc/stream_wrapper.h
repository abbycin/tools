/*********************************************************
          File Name:stream_wrapper.h
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 04:03:42 PM CST
**********************************************************/

#ifndef LOG_STREAM_H_
#define LOG_STREAM_H_

#include <cstring>
#include <limits>
#include <string>

namespace nm
{
  template<unsigned int N>
  class Buffer
  {
    public:
      Buffer(): size_(N), index_(0)
      {
        memset(buf_, 0, N);
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
          memcpy(buf_ + index_, line, len);
          index_ += len;
        }
      }
      int rest() const { return size_ - index_; }
      size_t index() const { return index_; }
      bool avail() const { return !(size_ == index_); }
      const char* data() { return buf_; }
      void reset()
      {
        index_ = 0;
      }
      std::string to_string() const
      {
        return std::string(buf_, index_);
      }
      const char* data() const
      {
        return buf_;
      }
    private:
      char buf_[N];
      size_t size_;
      size_t index_;
  };

  struct NullType {};
  template<typename T, typename U>
  struct TypeList
  {
    typedef T Head;
    typedef U Tail;
  };
#define TL1(T1) TypeList<T1, NullType>
#define TL2(T1, T2) TypeList<T1, TL1(T2)>
  template<typename> struct Max;
  template<> struct Max<NullType>
  {
    enum { value = 0 };
  };
  template<typename Head, typename Tail>
  struct Max<TypeList<Head, Tail>>
  {
    private:
      enum
      {
        cur = std::numeric_limits<Head>::digits10,
        nxt = Max<Tail>::value
      };
    public:
      enum { value = cur > nxt ? cur : nxt };
  };

  class StreamWrapper
  {
    private:
      enum
      {
        // openSUSE Tumbleweed with kernel-4.7 _x86_64 got (18 + 1) = 19
        BUF_SIZE = Max<TL2(long, long double)>::value + 1,
        BUFFER_SIZE = 1024 * 4 // 4KB
      };
    public:
      using mBuffer = Buffer<BUFFER_SIZE>;
      StreamWrapper();
      StreamWrapper(const StreamWrapper&) = delete;
      StreamWrapper(StreamWrapper&&) = delete;
      StreamWrapper& operator= (const StreamWrapper&) = delete;
      StreamWrapper& operator= (StreamWrapper&&) = delete;
      ~StreamWrapper();
      StreamWrapper& operator<< (const std::string&);
      StreamWrapper& operator<< (const mBuffer&);
      StreamWrapper& operator<< (const char*);
      StreamWrapper& operator<< (const int);
      StreamWrapper& operator<< (const unsigned int);
      StreamWrapper& operator<< (const long);
      StreamWrapper& operator<< (const unsigned long);
      StreamWrapper& operator<< (const float);
      StreamWrapper& operator<< (const double);
      const mBuffer& buffer() const { return buffer_; }
      void reset() { buffer_.reset(); }
    private:
      mBuffer buffer_;
      char buf_[BUF_SIZE];  // max numeric size, static_assert()
      template<typename T>
      StreamWrapper& append_(const char*, const T);
  };
}

#endif
