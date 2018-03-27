#include "_config.h"

# include <winbase.h>
# include <winsock2.h> // for struct timeval

#include <sys/types.h>

// This is close to the number of seconds in the time from the years
// 1601 to 1970 UNIX time starts on 1970.1.1 and Windows time starts on
// 1601.1.1
#define WIN_OFFSET ((1970-1601)*((LONGLONG)(365*24*3600+6*3600)))

int gettimeofday(struct timeval *t, void *p=NULL)
{
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  // t0 is now the number of 100 nano seconds = 1e-7 seconds
  // t->tv_sec will be the number of seconds
  // t->tv_usec will be the number of micro second = 1e-6 seconds
  
  unsigned int l = ft.dwLowDateTime;
  unsigned int h = ft.dwHighDateTime;
  
  LARGE_INTEGER li0 = { l, h };
  LARGE_INTEGER li1 = { l, h };
  
  li0.QuadPart /= 10000000;
  li1.QuadPart = li1.QuadPart - li0.QuadPart*10000000;
  li1.QuadPart /= 10;
  
  li0.QuadPart = li0.QuadPart - WIN_OFFSET;
  
  t->tv_sec = (time_t) li0.QuadPart;
  t->tv_usec = (unsigned int) li1.QuadPart;
  
  return 0;
}
