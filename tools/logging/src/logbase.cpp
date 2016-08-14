/*********************************************************
          File Name:logbase.cpp
          Author: Abby Cin
          Mail: abbytsing@gmail.com
          Created Time: Sun 14 Aug 2016 12:04:10 PM CST
**********************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

namespace nm
{
  int gettid()
  {
    return static_cast<int>(syscall(SYS_gettid));
  }
}
