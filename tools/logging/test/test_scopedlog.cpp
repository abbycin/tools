/*********************************************************
          File Name:test_scopedlog.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 11:21:14 AM CST
**********************************************************/

#include <iostream>
#include <exception>
#include "scopedlog.h"

FILE* fp = nullptr;

void out(const char* line, int len)
{
  fwrite(line, 1, len, fp);
}

void flush()
{
  fflush(fp);
}

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    fprintf(stderr, "%s logpath\n", argv[0]);
    return 1;
  }
  fp = fopen(argv[1], "a");
  nm::ScopedLog::set_appender(out);
  nm::ScopedLog::set_flush(flush);
  char buf[64 * 1024] = {0};
  setbuffer(fp, buf, 64 * 1024);
  const char* s = "abc";
  int n = 123;
  long l = 1992;
  unsigned int un = 123u;
  unsigned long ul = 1992ul;
  float f = 3.09f;
  double d = 19.8919920309f;
  for(int i = 0; i < 250000; ++i)
  {
    nm::LOGINFO << s << " " << n << " "
      << l << " " << un << " " << ul
      << " " << f << " " << d << "\n";
    nm::LOGDEBUG << s << " " << n << " "
      << l << " " << un << " " << ul
      << " " << f << " " << d << "\n";
    nm::LOGWARN << s << " " << n << " "
      << l << " " << un << " " << ul
      << " " << f << " " << d << "\n";
    nm::LOGERR << s << " " << n << " "
      << l << " " << un << " " << ul
      << " " << f << " " << d << "\n";
  }
  fclose(fp);
}
