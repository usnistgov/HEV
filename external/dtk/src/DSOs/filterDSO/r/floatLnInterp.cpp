/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */

// dtk.h defines DTK_ARCH_DARWIN or not
#include <dtk.h>

#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
#  define DBL_MIN __DBL_MIN__
#  define MINDOUBLE DBL_MIN
#elif defined DTK_ARCH_WIN32_VCPP
#  include <float.h>
#  define MINDOUBLE DBL_MIN
#  define MINFLOAT FLT_MIN
#else
#  include <values.h>
#endif

#include <math.h>
#include <errno.h>
#include <string.h>
#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif
#include <stdlib.h>

#include <dtk/dtkFilter_loader.h>

#define  LDBL_TIME(x) (((long double) (x)->tv_sec)  + \
                       ((long double) (x)->tv_usec) * ((long double) 1.0e-6))

#ifndef DTK_DOUBLE
# define DTK_FLOAT
#endif

#ifdef DTK_FLOAT
# define TYPE          float
# define TYPE_STR      "float"
# define OUTPUT_FILE   "floatLnInterp"
# define FILTER        floatLnInterp
#endif

#ifdef DTK_DOUBLE
# define TYPE          double
# define TYPE_STR      "double"
# define OUTPUT_FILE   "doubleLnInterp"
# define FILTER        doubleLnInterp
#endif

#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
// Someone needs to find a good number to put here.
#  define  SMALL_TIME    ((long double) -1.0e200)
#else
#  define  SMALL_TIME    ((long double) MINDOUBLE)
#endif

#define  SMALL_DELTA_T ((long double) 1.0e-7)

// default TIME_DELAY
#define TIME_DELAY         ((long double) 1/50.0)

// default QUEUE_LENGTH
#define QUEUE_LENGTH       64

#define MIN_QUEUE_LENGTH   2
#define MAX_QUEUE_LENGTH   2000

struct ValueList
{
  TYPE *x;
  long double t;
  u_int64_t count;
  struct ValueList *next;
  struct ValueList *prev;
};


class FILTER : public dtkFilter
{
public:

  FILTER(size_t size, int argc, const char **argv);
  ~FILTER(void);

  int read(void *buf, size_t bytes, size_t offset, int diffByteOrder,
	   struct timeval *timeStamp, u_int64_t count);
private:

  int Usage(void);
  int parse_args(size_t size, int argc, const char **argv, int *queue_length);
  int arraySize;
  // List of values from shared memory to interpolate on.
  struct ValueList *list, *a, *b, *last;
  long double t_delay;
  dtkTime Time;
};


int FILTER::Usage(void)
{
  dtkMsg.add(DTKMSG_NOTICE,
	     "\n"
	     "  Usage: "OUTPUT_FILE" [-l|--queue-length LENGTH]|[-t|--time-delay TIME_DELAY]\n"
	     "\n"
	     "  Load a read filter that does linear interpolation of the shared memory data by\n"
	     "interpreting the data as an array of "TYPE_STR"s using time as the independent\n"
	     "interpolation variable.  The interpolation time is the current time minus\n"
	     "TIME_DELAY in seconds.\n"
	     "\n"
	     "-l|--queue-length LENGTH       set the number of values that are buffered in order to find\n"
	     "                               the value with the time that is being used to interpolate on\n"
	     "                               to LENGTH   The default queue length is %d. The maximum queue\n"
	     "                               length allowed is %d.  The minimum queue length allowed is %d.\n"
	     " -t|--time-delay TIME_DELAY    set TIME_DELAY in seconds The default value for TIME_DELAY is\n"
	     "                               %Lg seconds.\n"
	     "\n",
	     QUEUE_LENGTH, MIN_QUEUE_LENGTH, MAX_QUEUE_LENGTH, TIME_DELAY);
  return -1;
}


int FILTER::parse_args(size_t size, int argc, const char **argv, int *list_length)
{
  int i;
  for(i=0;i<argc;i++)
    if(!strcmp(argv[i], "--help") || !strncmp(argv[i], "-h",2))
      return Usage();

  for(i=0;i<argc;)
    {
      char *str;
      if((str = dtk_getOpt("-t","--time-delay",argc,argv,&i)))
        {
#ifdef DTK_ARCH_WIN32_VCPP
	   t_delay = strtod(str, NULL);
#else
	   t_delay = strtold(str, NULL);
#endif
	   if(errno == ERANGE)
	     {
	       dtkMsg.add(DTKMSG_WARN, "shared memory read filter \""OUTPUT_FILE"\"\n"
			  "bad option \"-t|--time-delay\"\n");
	       return Usage();
	     }
        }
      else if((str = dtk_getOpt("-l","--queue-length",argc,argv,&i)))
	{
	  *list_length = atoi(str);
	   if(!(*list_length <= MAX_QUEUE_LENGTH || *list_length >= MIN_QUEUE_LENGTH))
	     {
	       dtkMsg.add(DTKMSG_WARN, "shared memory read filter \""OUTPUT_FILE"\"\n"
			  "bad option \"-l|--queue-length\"\n");
	       return Usage();
	     }
	}
      else
	{
	  dtkMsg.add(DTKMSG_WARN, "shared memory read filter \""OUTPUT_FILE"\"\n"
		     "bad option \"%s\"\n",argv[i]);
	  return Usage();
	}
    }
  return 0;
}


FILTER::FILTER(size_t size, int argc, const char **argv): dtkFilter(size)
{
  list = NULL;
  arraySize = size/sizeof(TYPE);
  if(arraySize < 1)
    {
      dtkMsg.add(DTKMSG_WARN, "The DTK shared memory is smaller than "
		 "a "TYPE_STR".\n");
      return; // error
    }
  int list_length = QUEUE_LENGTH;
  t_delay = TIME_DELAY;
  if(parse_args(size, argc, argv, &list_length)) 
    return; // error

  // Make a linked list for the values from shared memory.
  list = (struct ValueList *) dtk_malloc(list_length*sizeof(struct ValueList));
  int i=0;
  for(;i<list_length-1;i++)
    {
      list[i].next = &(list[i+1]);
      list[i+1].prev = &(list[i]);
      list[i].x = (TYPE *) dtk_malloc(arraySize*sizeof(TYPE));
      list[i].t = SMALL_TIME;
      list[i].count = 0;
    }
  list[0].prev = &(list[i]);
  list[i].next = list;
  list[i].x = (TYPE *) dtk_malloc(arraySize*sizeof(TYPE));
  list[i].t = SMALL_TIME;
  list[i].count = 0;

  a = b = NULL;
  last = list;

  requestTimeStamp();
  requestQueued();
  requestWriteCount();
  requestAutoByteSwap(sizeof(TYPE));

  dtkMsg.add(DTKMSG_DEBUG, "loaded filter \""OUTPUT_FILE"\" with "
	     "time_delay=%Lg and filter buffer length=%d\n",
	     t_delay, list_length);

  validate(); // success
}


FILTER::~FILTER(void)
{
  if(list)
  {
    struct ValueList *l = list;
    dtk_free(l->x);
    for(l = l->next;l!=list;l = l->next)
      dtk_free(l->x);
    dtk_free(list);
  }
}


int FILTER::read(void *buf, size_t bytes, size_t offset,
		 int diffByteOrder, struct timeval *tv, u_int64_t c)
{
  if(bytes != sizeof(TYPE)*arraySize || offset) return dtkFilter::REMOVE;

  long double t_ = Time.get();
  long double t = t_ - t_delay;

  // add the values to the list if they are new values.

  if(last->count != c)
    {
      last = last->next;
      last->count = c;
      last->t = LDBL_TIME(tv);
      memcpy(last->x, buf, bytes);
      if(!b) b = last;
    }

  if(a && b)
    {
      while(t > b->t && b != last)
	{
	  a = b;
	  b = b->next;
	}
      // We interpolate between the values a and b if the requested
      // time in t is between a->t and b->t.  We assume that the input
      // time, tv, is increasing.  The values must be consecutive, so
      // we use the count to make sure.
      TYPE *out = (TYPE *) buf;
      int i;
      if(t > a->t && t < b->t && a->count + 1 == b->count)
	{
	  long double dd = (t - a->t)/(b->t - a->t);
	  for(i=0;i<arraySize;i++)
	    out[i] = a->x[i] + (b->x[i] - a->x[i])*dd;
	}
      else
	{
	  for(i=0;i<arraySize;i++)
	    out[i] = b->x[i];
	}
      // else // b->t <= t
      /* leave the return buf alone. */
    }
  else // don't have two values of a and b saved yet.
    {
      a = last;
      b = NULL;
    }
  //printf("last->x[count=%Ld]=%g a->x=%g\n",last->count, last->x[0],a?a->x[0]:0.0f);
  

  // Return the time that we think we are interpolated with which is
  // the current time. The output is shifted in time by t_delay.
  tv->tv_sec = (long) t_;
  tv->tv_usec = (long) ((t_ - tv->tv_sec)*((long double) 1000000)); 

  return dtkFilter::CONTINUE; // success
}


/************** All dtkFilter DSOs define these two functions ****************
 *
 * If you wish to build your filter DSO from more than one file and
 * you wish to not define dtkFilter_loader and/or dtkFilter_unloader
 * in another source file then you may then define
 * __DTKFILTER_NOT_LOADED__ before including the
 * dtk/dtkFliter_loader.h in that source file.
 *
 *****************************************************************************/

static dtkFilter *dtkFilter_loader(size_t size, int argc, const char **args)
{
  return new FILTER(size, argc, args);
}

static int dtkFilter_unloader(dtkFilter *f)
{
  delete f;
  return DTKFILTER_UNLOAD_CONTINUE;
}
