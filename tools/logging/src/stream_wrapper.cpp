/*********************************************************
          File Name:stream_wrapper.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 07:03:27 PM CST
**********************************************************/

#include "stream_wrapper.h"

namespace nm
{
  BufferStream& BufferStream::operator<< (const std::string& s)
  {
    buffer_.append(s.c_str(), s.size());
    return *this;
  }

  FileStream& FileStream::operator<< (const std::string& s)
  {
    file_->append(s.c_str(), s.size());
    return *this;
  }
}
