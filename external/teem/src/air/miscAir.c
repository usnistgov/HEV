/*
  teem: Gordon Kindlmann's research software
  Copyright (C) 2003, 2002, 2001, 2000, 1999, 1998 University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "air.h"
#include <teem32bit.h>
/* timer functions */
#ifdef _WIN32
#include <fcntl.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#if TEEM_32BIT == 1
const int airMy32Bit = 1;
const char airMyFmt_size_t[] = "%u";
#else
const int airMy32Bit = 0;
const char airMyFmt_size_t[] = "%lu";
#endif

/*
******** airTeemVersion
******** airTeemReleaseDate
**
** updated with each release to contain a string representation of 
** the teem version number and release date.  Originated in version 1.5.
*/
const char *
airTeemVersion = "1.6";
const char *
airTeemReleaseDate = "28 Oct 2003";

/*
******** airSrand()
**
** uses clock to seed srand()
*/
void
airSrand(void) {
  
  srand((unsigned int)airTime());
}

/*
******** airRand()
**
** returns double in range [0.0,1.0)
*/
double
airRand(void) {
  double val, sc = 1.0/(RAND_MAX+1.0);

  val = sc*rand();

  /* repeat as needed */
  val = sc*(val + rand());
  val = sc*(val + rand());
  val = sc*(val + rand());

  return val;
}

/*
******** airRandInt
**
** returns a random integer in range [0, N-1]
*/
int
airRandInt(int N) {
  int i;
  
  AIR_INDEX(0.0, airRand(), 1.0, N, i);
  i = AIR_CLAMP(0, i, N-1);
  return i;
}

/*
******** airShuffle()
**
** generates a random permutation of integers [0..N-1] if perm is non-zero,
** otherwise, just fills buff with [0..N-1] in order
*/
void
airShuffle(int *buff, int N, int perm) {
  int i, swp, tmp;

  if (!(buff && N > 0))
    return;
    
  for (i=0; i<N; i++)
    buff[i] = i;
  if (perm) {
    for (i=0; i<N; i++) {
      swp = i + airRandInt(N - i);
      tmp = buff[swp];
      buff[swp] = buff[i];
      buff[i] = tmp;
    }
  }
}

/*
******** airNull()
**
** returns NULL
*/
void *
airNull(void) {

  return NULL;
}

/*
******** airSetNull
**
** dereferences and sets to NULL, returns NULL
*/
void *
airSetNull(void **ptrP) {
  
  if (ptrP) {
    *ptrP = NULL;
  }
  return NULL;
}

/*
******** airFree()
**
** to facilitate setting a newly free()'d pointer; always returns NULL.
** also makes sure that NULL is not passed to free().
*/
void *
airFree(void *ptr) {

  if (ptr) {
    free(ptr);
  }
  return NULL;
}

/*
******** airFreeP()
**
** calls airFree on the address pointed to by the argument
*/
void *
airFreeP(void *_ptrP) {
  void **ptrP;

  ptrP = _ptrP;
  if (ptrP)
    airFree(*ptrP);
  return NULL;
}

/*
******** airFopen()
**
** encapsulates that idea that "-" is either standard in or stardard
** out, and does McRosopht stuff required to make piping work 
**
** Does not error checking.  If fopen fails, then C' errno and strerror are
** left untouched for the caller to access.
*/
FILE *
airFopen(const char *name, FILE *std, const char *mode) {
  FILE *ret;

  if (!strcmp(name, "-")) {
    ret = std;
#ifdef _WIN32
    if (strchr(mode, 'b')) {
      _setmode(_fileno(ret), _O_BINARY);
    }
#endif
  } else {
    ret = fopen(name, mode);
  }
  return ret;
}


/*
******** airFclose()
**
** just to facilitate setting a newly fclose()'d file pointer to NULL
** also makes sure that NULL is not passed to fclose(), and won't close
** stdin, stdout, or stderr (its up to the user to open these correctly)
*/
FILE *
airFclose(FILE *file) {

  if (file) {
    if (!( stdin == file || stdout == file || stderr == file )) {
      fclose(file);
    }
  }
  return NULL;
}

/*
******* airDoneStr()
**
** dinky little utility for generating progress messages of the form
** "  1.9%" or " 35.3%" or  "100.0%"
**
** The message will ALWAYS be six characters, and will ALWAYS be
** preceeded by six backspaces.  Thus, you pass in a string to print
** into, and it had better be allocated for at least 6+6+1 = 13 chars.
*/
char *
airDoneStr(float start, float here, float end, char *str) {
  int perc=0;

  if (str) {
    if (end != start)
      perc = (int)(1000*(here-start)/(end-start) + 0.5);
    else
      perc = 1000;
    if (perc < 0) {
      sprintf(str, "\b\b\b\b\b\b ---- ");
    } else if (perc < 1000) {
      sprintf(str, "\b\b\b\b\b\b% 3d.%d%%", perc/10, perc%10);
    }
    else if (perc == 1000) {
      /* the "% 3d" formatting sequence should have taken care
	 of this, but whatever */
      sprintf(str, "\b\b\b\b\b\b100.0%%");
    }
    else {
      sprintf(str, "\b\b\b\b\b\b  done");
    }
  }

  /* what the heck was all this for ? 
  static int len = -1;

  if (-1 == len) {
    len = strlen(str);
  }
  else {
    if (len != strlen(str)) {
      printf("len(\\b\\b\\b\\b\\b\\b% 3d.%d%%) != %d\n", 
	      perc/10, perc%10, len);
      exit(1);
    }
  }
  */
  return(str);
}

/*
******** airTime()
**
** returns current time in seconds (with millisecond resolution) as a double
*/
double
airTime() {
#ifdef _WIN32
  return (double)clock()/CLOCKS_PER_SEC;
#else
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz); 
  return((double)(tv.tv_sec + tv.tv_usec/1000000.0));
#endif
}

double
airSgnPow(double v, double p) {

  return (v >= 0
	  ? pow(v, p)
	  : -pow(-v, p));
}

/*
******** airLog2()
**
** silly little function which returns log_2(n) if n is a power of 2,
** or -1 otherwise
*/
int 
airLog2(float n) {

  if (!AIR_EXISTS(n))
    return -1;
  if (n == 1.0)
    return 0;
  if (n < 2)
    return -1;
  return airLog2(n/2.0);
}

int
airSgn(double v) {
  return (v > 0
	  ? 1
	  : (v < 0
	     ? -1
	     : 0));
}

/*
******** airCbrt
**
** cbrt() isn't ANSI, so any hacks to create a stand-in for cbrt()
** are done here.
*/
double
airCbrt(double v) {
#ifdef _WIN32
  return pow(v,1.0/3.0);
#else
  return cbrt(v);
#endif
}

/*
******** airSinglePrintf
**
** a complete stand-in for {f|s}printf(), as long as the given format
** string contains exactly one conversion sequence.  The utility of
** this is to standardize the printing of IEEE 754 special values:
** QNAN, SNAN -> "NaN"
** POS_INF -> "+inf"
** NEG_INF -> "-inf"
** The format string can contain other things besides just the
** conversion sequence: airSingleFprintf(f, " (%f)\n", AIR_NAN)
** will be the same as fprintf(f, " (%s)\n", "NaN");
**
** To get fprintf behavior, pass "str" as NULL
** to get sprintf bahavior, pass "file" as NULL
**
** Someday I'll find/write a complete {f|s|}printf replacement ...
*/
int
airSinglePrintf(FILE *file, char *str, const char *_fmt, ...) {
  char *fmt;
  float valF=0;
  double valD=0;
  int ret, isF, isD, cls;
  char *conv=NULL, *p0, *p1, *p2, *p3, *p4, *p5;
  va_list ap;
  
  va_start(ap, _fmt);
  fmt = airStrdup(_fmt);
  
  p0 = strstr(fmt, "%e");
  p1 = strstr(fmt, "%f");
  p2 = strstr(fmt, "%g");
  p3 = strstr(fmt, "%le");
  p4 = strstr(fmt, "%lf");
  p5 = strstr(fmt, "%lg");
  isF = p0 || p1 || p2;
  isD = p3 || p4 || p5;
  if (isF) {
    conv = p0 ? p0 : (p1 ? p1 : p2);
  }
  if (isD) {
    conv = p3 ? p3 : (p4 ? p4 : p5);
  }
  if (isF || isD) {
    if (isF) {
      /* use "double" instead of "float" because var args are _always_
	 subject to old-style C type promotions: float promotes to double */
      valF = (float)(va_arg(ap, double));
      cls = airFPClass_f(valF);
    }
    else {
      valD = va_arg(ap, double);
      cls = airFPClass_d(valD);
    }
    switch (cls) {
    case airFP_SNAN:
    case airFP_QNAN:
    case airFP_POS_INF:
    case airFP_NEG_INF:
      if (isF) {
	memcpy(conv, "%s", 2);
      }
      else {
	/* this sneakiness allows us to replace a 3-character conversion
	   sequence for a double (such as %lg) with a 3-character conversion
	   for a string, which we know has at most 4 characters */
	memcpy(conv, "%4s", 3);
      }
      break;
    }
#define PRINT(F, S, C, V) ((F) ? fprintf((F),(C),(V)) : sprintf((S),(C),(V)))
    switch (cls) {
    case airFP_SNAN:
    case airFP_QNAN:
      ret = PRINT(file, str, fmt, "NaN");
      break;
    case airFP_POS_INF:
      ret = PRINT(file, str, fmt, "+inf");
      break;
    case airFP_NEG_INF:
      ret = PRINT(file, str, fmt, "-inf");
      break;
    default:
      if (isF) {
	ret = PRINT(file, str, fmt, valF);
      }
      else {
	ret = PRINT(file, str, fmt, valD);
      }
      break;
    }
  }
  else {
    ret = file ? vfprintf(file, fmt, ap) : vsprintf(str, fmt, ap);
  }
  
  va_end(ap);
  free(fmt);
  return ret;
}

void
airBinaryPrintUInt(FILE *file, int digits, unsigned int N) {

  digits = AIR_CLAMP(1, digits, 32);
  for (digits=digits; digits>=1; digits--) {
    fprintf(file, "%c", ((1<<(digits-1)) & N
			 ? '1' : '0'));
  }
}

double
airErf(double x) {
  /* 
   * When I was a Cornell undergrad (sophomore year, 1992), I was
   * interested in a programming job in the Astronomy department.  The
   * job posting said I should talk to Saul Teukolsky, one of the
   * Numerical Recipes authors.  The first thing he asked was "What's
   * your GPA?".  I was confused.  If programming ability is the real
   * question at hand, this was about as sensible as asking "How much
   * do you weigh?"  As soon as I admitted to getting a B+ in a
   * previous physics class, he literally just waved me away and
   * turned back to his computer.  
   */
  double t,z,ans;

  z = AIR_ABS(x);
  t = 1.0/(1.0+0.5*z);
  ans=t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+
            t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+
            t*(-0.82215223+t*0.17087277)))))))));
  return (x >= 0.0 ? 1.0 - ans : ans - 1.0);
}

double
airGaussian(double x, double mean, double stdv) {
  
  x = x - mean;
  return exp(-(x*x)/(2*stdv*stdv))/(stdv*sqrt(2*M_PI));
}

const char
airTypeStr[AIR_TYPE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown)",
  "bool",
  "int",
  "float",
  "double",
  "char",
  "string",
  "enum",
  "other",
};

const int
airTypeSize[AIR_TYPE_MAX+1] = {
  0,
  (int)sizeof(int),
  (int)sizeof(int),
  (int)sizeof(float),
  (int)sizeof(double),
  (int)sizeof(char),
  (int)sizeof(char*),
  (int)sizeof(int),
  0   /* we don't know anything about type "other" */
};

int
airILoad(void *v, int t) {

  switch(t) {
  case airTypeBool:   return *((int*)v); break;
  case airTypeInt:    return *((int*)v); break;
  case airTypeFloat:  return *((float*)v); break;
  case airTypeDouble: return *((double*)v); break;
  case airTypeChar:   return *((char*)v); break;
  default: return 0; break;
  }
}

int
airIStore(void *v, int t, int i) {

  switch(t) {
  case airTypeBool:   return (*((int*)v) = !!i); break;
  case airTypeInt:    return (*((int*)v) = i); break;
  case airTypeFloat:  return (*((float*)v) = i); break;
  case airTypeDouble: return (*((double*)v) = i); break;
  case airTypeChar:   return (*((char*)v) = i); break;
  default: return 0; break;
  }
}

float
airFLoad(void *v, int t) {

  switch(t) {
  case airTypeBool:   return *((int*)v); break;
  case airTypeInt:    return *((int*)v); break;
  case airTypeFloat:  return *((float*)v); break;
  case airTypeDouble: return *((double*)v); break;
  case airTypeChar:   return *((char*)v); break;
  default: return 0; break;
  }
}

float
airFStore(void *v, int t, float f) {

  switch(t) {
  case airTypeBool:   return (*((int*)v) = f); break;
  case airTypeInt:    return (*((int*)v) = f); break;
  case airTypeFloat:  return (*((float*)v) = f); break;
  case airTypeDouble: return (*((double*)v) = f); break;
  case airTypeChar:   return (*((char*)v) = f); break;
  default: return 0; break;
  }
}

double
airDLoad(void *v, int t) {

  switch(t) {
  case airTypeBool:   return *((int*)v); break;
  case airTypeInt:    return *((int*)v); break;
  case airTypeFloat:  return *((float*)v); break;
  case airTypeDouble: return *((double*)v); break;
  case airTypeChar:   return *((char*)v); break;
  default: return 0; break;
  }
}

double
airDStore(void *v, int t, double d) {

  switch(t) {
  case airTypeBool:   return (*((int*)v) = d); break;
  case airTypeInt:    return (*((int*)v) = d); break;
  case airTypeFloat:  return (*((float*)v) = d); break;
  case airTypeDouble: return (*((double*)v) = d); break;
  case airTypeChar:   return (*((char*)v) = d); break;
  default: return 0; break;
  }
}

