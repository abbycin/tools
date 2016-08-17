/*********************************************************
          File Name:stream_wrapper.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 07:03:27 PM CST
**********************************************************/

#include "meta/stream_wrapper.h"

namespace nm
{
  namespace meta
  {
    BufferStream& BufferStream::operator<< (const std::string& s)
    {
      buffer_.append(s.c_str(), s.size());
      return *this;
    }
    void BufferStream::append(const char* s, const size_t len)
    {
      buffer_.append(s, len);
    }
    size_t BufferStream::size() const
    {
      return buffer_.size();
    }
    void BufferStream::reset()
    {
      buffer_.reset();
    }
    void BufferStream::flush()
    {
      buffer_.clear();
    }

    FileStream& FileStream::operator<< (const std::string& s)
    {
      file_->append(s.c_str(), s.size());
      return *this;
    }
    void FileStream::set(FileCtl* f)
    {
      file_ = f;
    }
    bool FileStream::valid()
    {
      return file_ != nullptr;
    }
    void FileStream::flush()
    {
      file_->flush();
    }
    void FileStream::append(const char* s, size_t len)
    {
      file_->append(s, len);
    }
  }
}
