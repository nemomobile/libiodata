#include <cstdio>
#include <cstdarg>
#include <string>

using namespace std ;

#include "misc.h"
#include "log.h"

string str_vprintf(const char *format, va_list varg)
{
  const int buf_len = 1024, max_buf_len = buf_len*1024 ;
  char buf[buf_len], *p = buf ;
  int iteration = 0, printed = false ;
  string formatted ;
  do
  {
    int size = buf_len << iteration ;
    if(size>max_buf_len)
    {
      log_error("Can't format string, the result is too long") ;
      return format ;
    }
    if(iteration>0)
      p = new char[size] ;
    int res = vsnprintf(p, size, format, varg) ;
    if(res < 0)
    {
      log_error("Can't format string, vsnprintf() failed") ;
      return format ;
    }
    if(res < size)
    {
      printed = true ;
      formatted = p ;
    }
    if(iteration > 0)
      delete[] p ;
    ++ iteration ;
  } while(not printed) ;

  return formatted ;
}

