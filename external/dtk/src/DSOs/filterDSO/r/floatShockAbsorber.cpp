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
 */

// dtk.h defines DTK_ARCH_DARWIN or not
#include <dtk.h>

#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
#  define MAXFLOAT FLT_MAX
#  define MINFLOAT FLT_MIN
#  define MAXDOUBLE DBL_MAX
#  define MINDOUBLE DBL_MIN
#elif defined DTK_ARCH_WIN32_VCPP
#  include <float.h>
#  define MAXFLOAT FLT_MAX
#  define MINFLOAT FLT_MIN
#  define MAXDOUBLE DBL_MAX
#  define MINDOUBLE DBL_MIN
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
#include <dtk/dtkDiffEq.h>
#include <dtk/dtkRK4.h>

#define  LDBL_TIME(x) (((long double) (x)->tv_sec)  + \
                       ((long double) (x)->tv_usec) * ((long double) 1.0e-6))

#ifndef DTK_DOUBLE
# define DTK_FLOAT
#endif

#ifdef DTK_FLOAT
# define TYPE          float
# define TYPE_STR      "float"
# define VAL_MAX           MAXFLOAT
# define VAL_MIN           MINFLOAT
# define FORMAT        "%g"
# define OUTPUT_FILE   "floatShockAbsorber"
# define FILTER        floatShockAbsorber
#endif

#ifdef DTK_DOUBLE
# define TYPE          double
# define TYPE_STR      "double"
# define VAL_MAX           MAXDOUBLE
# define VAL_MIN           MINDOUBLE
# define FORMAT        "%g"
# define OUTPUT_FILE   "doubleShockAbsorber"
# define FILTER        doubleShockAbsorber
#endif

#define  SMALL_TIME    ((long double) MINDOUBLE)

#define  DEFAULT_T     ((TYPE) 0.2) // in seconds


class ODEs : public dtkDiffEq<TYPE>
{
public:
  ODEs(int numberOfEquations);
  virtual ~ODEs(void);

  void getDerivatives(TYPE *xdot, const TYPE *x, long double t) const;
  void setParam(TYPE T, int i);
  TYPE getT(int i);
  TYPE getMinT(void);

  TYPE* x0;
private:

  TYPE* param1;
  TYPE* param2;

  int numFilters;
};

ODEs::ODEs(int num)
{
  numFilters = num;
  numberOfEquations = 2*num;
  x0 = NULL;
  param1 = NULL;
  param2 = NULL;

  if(numberOfEquations < 1) return; // error

  x0 =     (TYPE *) dtk_malloc(numFilters*sizeof(TYPE));
  param1 = (TYPE *) dtk_malloc(numFilters*sizeof(TYPE));
  param2 = (TYPE *) dtk_malloc(numFilters*sizeof(TYPE));

  // set default parameters
  int i;
  for(i=0;i<numFilters;i++)
    {
      x0[i] = (TYPE) 0.0;
      setParam(DEFAULT_T,i);
    }
}

void ODEs::setParam(TYPE T, int i)
{
  param1[i] = - 1.0/(T*T);
  param2[i] = - 2.0/T;
}

TYPE ODEs::getT(int i)
{
  return -2/param2[i];
}

TYPE ODEs::getMinT(void)
{
  TYPE minT = VAL_MAX;
  int i;
  for(i=0;i<numFilters;i++)
    if(-2/param2[i] < minT)
      minT = -2/param2[i];
  return minT;
}

ODEs::~ODEs(void)
{
  if(x0)
    {
      dtk_free(x0);
      x0 = NULL;
    }
  if(param1)
    {
      dtk_free(param1);
      param1 = NULL;
    }
  if(param2)
    {
      dtk_free(param2);
      param2 = NULL;
    }
}

void ODEs::
getDerivatives(TYPE *xdot, const TYPE *x, long double t) const
{
  int i;
  for(i=0;i<numFilters;i++)
    {
      //xdot[i] = -(x[i] - x0[i]);
      xdot[i] = x[i+numFilters];
      xdot[i+numFilters] = param1[i]*(x[i]-x0[i]) + param2[i]*x[i+numFilters];
    }
}


class FILTER : public dtkFilter
{
public:

  FILTER(size_t size, int argc, const char **argv);
  ~FILTER(void);

  int read(void *buf, size_t bytes, size_t offset, int diffByteOrder,
	   struct timeval *timeStamp, u_int64_t count);
private:

  int Usage(void);
  int parse_args(int argc, const char **argv);
  int arraySize;
  ODEs odes;
  dtkRK4<TYPE, double> rk4;
  TYPE *x;
  long double last_t;
  long double delta_t_max;
  dtkTime *time;
};


int FILTER::Usage(void)
{
  dtkMsg.add(DTKMSG_NOTICE,
	     "\n"
	     "  Usage: "OUTPUT_FILE" [-s|--time-scale T]|[-S|--time-scales T0 T1 ...]\\\n"
	     "                                  [-t|--use-time-stamp]\n"
	     "\n"
	     "  Load a read filter that uses the shared memory data as a parametric input into a\n"
	     "second order ordinary differential equation for each "TYPE_STR" using time in seconds\n"
	     "as the independent variable. The values from share memory is the equilibrium position of\n"
	     "a critically damped simple harmonic oscillator.  The ODE can be written as:\n"
	     " d^2x/dt^2 = - (x - x0)*(1/T)^2 - (2/T)*dx/dt\n"
	     "where x is the dependent variable, the parameter T is the time in seconds that it takes\n"
	     "the value of x to go to 1/e times it's initial value, where e is Euler's constant, and\n"
	     "x0 is the equilibrium value of x.\n"
	     "\n"
	     " -s|--time-scale T          set all the time scale parameters to T\n"
	     " -S|--time-scales T0 T1 ... set the time scales to T0 for the first "TYPE_STR" in the array\n"
	     "                            and T1 for the second "TYPE_STR" in the array and so on\n"
	     " -t|--use-time-stamp        uses the shared memory time stamp as the independent integration\n"
	     "                            varaible.  By default system clock time at the time of the read is\n"
	     "                            used.\n"
	     "\n");
  return -1;
}


int FILTER::parse_args(int argc, const char **argv)
{
  int i;
  for(i=0;i<argc;i++)
    if(!strcmp(argv[i], "--help") || !strncmp(argv[i], "-h",2))
      return Usage();

  for(i=0;i<argc;)
    {
      char *str;
      if((str = dtk_getOpt("-s","--time-scale",argc,argv,&i)))
        {
#ifdef DTK_ARCH_WIN32_VCPP
	   TYPE T = (TYPE) strtod(str, NULL);
#else
	   TYPE T = (TYPE) strtold(str, NULL);
#endif
	   if(errno == ERANGE)
	     {
	       dtkMsg.add(DTKMSG_WARN, "shared memory read filter \""OUTPUT_FILE"\"\n"
			  "bad option \"-s|--time-scale\"\n");
	       return Usage();
	     }
	   int j;
	   for(j=0;j<arraySize;j++)
	     odes.setParam(T,j); 
        }
      else if((str = dtk_getOpt("-S","--time-scales",argc,argv,&i)))
        {
	  int j;
	  for(j=0;str && j<arraySize;str = (char *) argv[i++])
	    {
#ifdef DTK_ARCH_WIN32_VCPP
	      TYPE T = (TYPE) strtod(str, NULL);
#else
	      TYPE T = (TYPE) strtold(str, NULL);
#endif
	      if(errno == ERANGE)
		{
		  dtkMsg.add(DTKMSG_WARN, "shared memory read filter \""OUTPUT_FILE"\"\n"
			     "bad option \"-S|--time-scales\"\n");
		  return Usage();
		}
	       odes.setParam(T,j++);
	    }
        }
      else if(!strcmp("-t",argv[i]) || !strcmp("--use-time-stamp", argv[i]))
        {
	  requestTimeStamp();
	  requestQueued();
	  i++;
 	}
      else
	{
	  dtkMsg.add(DTKMSG_WARN, "shared memory read filter \""OUTPUT_FILE"\"\n"
		     "bad option \"%s\"\n",argv[i]);
	  return Usage();
	}
    }

  if(!needTimeStamp())
    time = new dtkTime;

  return 0;
}


FILTER::FILTER(size_t size, int argc, const char **argv):
  dtkFilter(size),
  arraySize(size/sizeof(TYPE)),
  odes(size/sizeof(TYPE)),
  rk4(&odes, (double) DEFAULT_T/50)
{
  x = NULL;
  time = NULL;
  last_t = (long double) VAL_MIN; 

  if(arraySize < 1)
    {
      dtkMsg.add(DTKMSG_WARN, "The DTK shared memory is smaller than "
		 "a "TYPE_STR".\n");
      return; // error
    }
  if(parse_args(argc, argv)) return; // error

  // There are 2 ODEs for each TYPE variable in shared memory.
  x = (TYPE *) dtk_malloc(2*arraySize*sizeof(TYPE));

  // We need to set some limit on how much time we integrate the ODEs
  // in a given read() call.  If the time increases more than delta_t_max
  // then we reset to the initial conditions.
  delta_t_max = odes.getMinT()*100;

  rk4.setTimeStep(odes.getMinT()/10);
  requestAutoByteSwap(sizeof(TYPE));


  dtkMsg.add(DTKMSG_DEBUG, "loaded filter \""OUTPUT_FILE"\"");
  if(time)
    dtkMsg.append(" with system time values, ");
  else
    dtkMsg.append(" with time values from shared memory, ");
  dtkMsg.append("with time scale(s):\n");
  int i;
  for(i=0;i<arraySize;i++)
    dtkMsg.append(" "FORMAT, odes.getT(i));
  dtkMsg.append("\n");

  validate(); // success
}


FILTER::~FILTER(void)
{
  if(x)
  {
    dtk_free(x);
    x = NULL;
  }
  if(time)
    {
      delete time;
      time = NULL;
    }
}


int FILTER::read(void *buf, size_t bytes, size_t offset,
		 int diffByteOrder, struct timeval *tv, u_int64_t c)
{
  if(bytes != sizeof(TYPE)*arraySize || offset) return dtkFilter::REMOVE;
  long double t;

  // If making my own time
  if(time) t =  time->get();
  // If getting time from incomming
  else t =  LDBL_TIME(tv);

  TYPE *out = (TYPE *) buf;
  if(last_t != (long double) VAL_MIN && (t - last_t < delta_t_max))
    {
      int i;
      for(i=0;i<arraySize;i++)
	odes.x0[i] = out[i];
      rk4.go(x, t, last_t);
      memcpy(out, x, bytes);
    }
  else // reset to the equilibrium positions for re-initialization.
    {
      int i;
      for(i=0;i<arraySize;i++)	  
	x[i] = odes.x0[i] = out[i];
      for(;i<2*arraySize;i++)
	x[i] = (TYPE) 0.0;

      rk4.setCurrentTime(t);
    }
  last_t = t;

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
