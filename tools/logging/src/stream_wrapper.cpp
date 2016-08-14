/*********************************************************
          File Name:stream_wrapper.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sat 13 Aug 2016 07:03:27 PM CST
**********************************************************/

#include "stream_wrapper.h"

namespace nm
{
  StreamWrapper::StreamWrapper(): buffer_()
  {
  }
  StreamWrapper::~StreamWrapper() {}
  StreamWrapper& StreamWrapper::operator<< (const std::string& s)
  {
    buffer_.append(s.c_str(), s.size());
    return *this;
  }
  StreamWrapper& StreamWrapper::operator<< (const mBuffer& buf)
  {
    return (*this << buf.to_string());
  }
  StreamWrapper& StreamWrapper::operator<< (const char* s)
  {
    buffer_.append(s, strlen(s));
    return *this;
  }
  StreamWrapper& StreamWrapper::operator<< (const int n)
  {
    return append_("%d", n);
  }
  StreamWrapper& StreamWrapper::operator<< (const unsigned int n)
  {
    return append_("%u", n);
  }
  StreamWrapper& StreamWrapper::operator<< (const long n)
  {
    return append_("%ld", n);
  }
  StreamWrapper& StreamWrapper::operator<< (const unsigned long n)
  {
    return append_("%lu", n);
  }
  StreamWrapper& StreamWrapper::operator<< (const float n)
  {
    return append_("%.6f", n);
  }
  StreamWrapper& StreamWrapper::operator<< (const double n)
  {
    return append_("%.10g", n);
  }
  template<typename T>
  StreamWrapper& StreamWrapper::append_(const char* fmt, const T x)
  {
    int len = snprintf(buf_, sizeof buf_, fmt, x);
    buffer_.append(buf_, len);
    return *this;
  }
}
