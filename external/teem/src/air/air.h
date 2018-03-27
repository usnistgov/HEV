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

#ifndef AIR_HAS_BEEN_INCLUDED
#define AIR_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define air_export __declspec(dllimport)
#else
#define air_export
#endif

#ifdef _WIN32
#if _MSC_VER < 1300 || !defined(_USE_MATH_DEFINES)
#define M_PI 3.14159265358979323846
#define M_E  2.71828182845904523536
#endif
typedef signed __int64 airLLong;
typedef unsigned __int64 airULLong;
#define AIR_LLONG_FMT "%I64d"
#define AIR_ULLONG_FMT "%I64u"
#define AIR_LLONG(x) x##i64
#define AIR_ULLONG(x) x##ui64
#else
typedef signed long long airLLong;
typedef unsigned long long airULLong;
#define AIR_LLONG_FMT "%lld"
#define AIR_ULLONG_FMT "%llu"
#define AIR_LLONG(x) x##ll
#define AIR_ULLONG(x) x##ull
#endif

#define AIR_STRLEN_SMALL (128+1)
#define AIR_STRLEN_MED   (256+1)
#define AIR_STRLEN_LARGE (512+1)
#define AIR_STRLEN_HUGE  (1024+1)

/* enum.c: enum value <--> string conversion utility */  
typedef struct {
  char name[AIR_STRLEN_SMALL];
               /* what are these things? */
  int M;       /* If "val" is NULL, the the valid enum values are from 1 to M
 	          (represented by strings str[1] through str[M]), and the
 	          unknown/invalid value is 0.  If "val" is non-NULL, the
 	          valid enum values are from val[1] to val[M] (but again, 
		  represented by strings str[1] through str[M]), and the
		  unknown/invalid value is val[0].  In both cases, str[0]
		  is the string to represent an unknown/invalid value */
  char (*str)[AIR_STRLEN_SMALL]; 
               /* "canonical" textual representation of the enum values */
  int *val;    /* non-NULL iff valid values in the enum are not [1..M], and/or
		  if value for unknown/invalid is not zero */
  char (*desc)[AIR_STRLEN_MED];
               /* desc[i] is a short description of the enum values represented
		  by str[i] (thereby starting with the unknown value), to be
		  used to by things like hest */
  char (*strEqv)[AIR_STRLEN_SMALL];  
               /* All the variations in strings recognized in mapping from
		  string to value (the values in valEqv).  This **MUST** be
		  terminated by a zero-length string ("") so as to signify
		  the end of the list.  This should not contain the string
		  for unknown/invalid.  If "strEqv" is NULL, then mapping
		  from string to value is done by traversing "str", and 
		  "valEqv" is ignored. */
  int *valEqv; /* The values corresponding to the strings in strEqv; there
		  should be one integer for each non-zero-length string in
		  strEqv: strEqv[i] is a valid string representation for
		  value valEqv[i]. This should not contain the value for
		  unknown/invalid.  This "valEqv" is ignored if "strEqv" is
		  NULL. */
  int sense;   /* require case matching on strings */
} airEnum;
extern int airEnumUnknown(airEnum *enm);
extern int airEnumValCheck(airEnum *enm, int val);
extern char *airEnumStr(airEnum *enm, int val);
extern char *airEnumDesc(airEnum *enm, int val);
extern int airEnumVal(airEnum *enm, const char *str);
extern char *airEnumFmtDesc(airEnum *enm, int val, int canon, const char *fmt);

/*
******** airEndian enum
**
** for identifying how a file was written to disk, for those encodings
** where the raw data on disk is dependent on the endianness of the
** architecture.
*/
enum {
  airEndianUnknown,         /* 0: nobody knows */
  airEndianLittle = 1234,   /* 1234: Intel and friends */
  airEndianBig = 4321,      /* 4321: the rest */
  airEndianLast
};
/* endianAir.c */
extern air_export airEnum *airEndian;
extern air_export const int airMyEndian;

/* array.c: poor-man's dynamically resizable arrays */
typedef struct {
  void *data,         /* where the data is */
    **dataP;          /* (possibly NULL) address of user's data variable,
			 kept in sync with internal "data" variable */
  int len,            /* length of array: # units for which there is
		         considered to be data (which is <= total # units
		         allocated).  The # bytes which contain data is
		         len*unit.  Always updated (unlike "*lenP") */
    *lenP,            /* (possibly NULL) address of user's length variable,
			 kept in sync with internal "len" variable */
    incr,             /* the granularity of the changes in amount of space
			 allocated: when the length reaches a multiple of
			 "incr", then the array is resized */
    size;             /* array is allocated to have "size" increments, or,
			 size*incr elements, or, 
			 size*incr*unit bytes */
  size_t unit;        /* the size in bytes of one element in the array */

  /* the following are all callbacks useful for maintaining either an array
     of pointers (allocCB and freeCB) or array of structs (initCB and
     doneCB).  allocCB or initCB is called when the array length increases,
     and freeCB or doneCB when it decreases.  Any of them can be NULL if no
     such activity is desired.  allocCB sets values in the array (as in
     storing the return from malloc(); freeCB is called on values in the
     array (as in calling free()), and the values are cast to void*.  allocCB
     and freeCB don't care about the value of "unit" (though perhaps they
     should).  initCB and doneCB are called on the _addresses_ of elements in
     the array.  allocCB and initCB are called for the elements in ascending
     order in the array, and freeCB and doneCB are called in descending
     order.  allocCB and initCB are mutually exclusive- they can't both be
     non-NULL. Same goes for freeCB and doneCB */
  void *(*allocCB)(void);  /* values of new elements set to return of this */
  void *(*freeCB)(void *); /* called on the values of invalidated elements */
  void (*initCB)(void *);  /* called on addresses of new elements */
  void (*doneCB)(void *);  /* called on addresses of invalidated elements */

} airArray;
extern airArray *airArrayNew(void **dataP, int *lenP, size_t unit, int incr);
extern void airArrayStructCB(airArray *a, 
			     void (*initCB)(void *), void (*doneCB)(void *));
extern void airArrayPointerCB(airArray *a, 
			      void *(*allocCB)(void), void *(*freeCB)(void *));
extern int airArraySetLen(airArray *a, int newlen);
extern int airArrayIncrLen(airArray *a, int delta);
extern airArray *airArrayNix(airArray *a);
extern airArray *airArrayNuke(airArray *a);

/* threadAir.c: simplistic wrapper functions for multi-threading  */
/*
********  airThreadCapable
**
** if non-zero: we have some kind of multi-threading available, either
** via pthreads, or via Windows stuff
*/
extern air_export const int airThreadCapable;

/*
******** airThreadNoopWarning
**
** When multi-threading is not available, and hence constructs like
** mutexes are not available, the operations on them will be
** no-ops. When this variable is non-zero, we fprintf(stderr) a
** warning to this effect when those constructs are used
*/
extern air_export int airThreadNoopWarning; 

/* opaque typedefs for OS-specific stuff */
typedef struct _airThread airThread;
typedef struct _airThreadMutex airThreadMutex;
typedef struct _airThreadCond airThreadCond;
typedef struct {
  unsigned int numUsers, numDone;
  airThreadMutex *doneMutex;
  airThreadCond *doneCond;
} airThreadBarrier;

extern airThread *airThreadNew(void);
extern int airThreadStart(airThread *thread, 
			  void *(*threadBody)(void *), void *arg);
extern int airThreadJoin(airThread *thread, void **retP);
extern airThread *airThreadNix(airThread *thread);

extern airThreadMutex *airThreadMutexNew();
extern int airThreadMutexLock(airThreadMutex *mutex);
extern int airThreadMutexUnlock(airThreadMutex *mutex);
extern airThreadMutex *airThreadMutexNix(airThreadMutex *mutex);

extern airThreadCond *airThreadCondNew();
extern int airThreadCondWait(airThreadCond *cond, airThreadMutex *mutex);
extern int airThreadCondSignal(airThreadCond *cond);
extern int airThreadCondBroadcast(airThreadCond *cond);
extern airThreadCond *airThreadCondNix(airThreadCond *cond);

extern airThreadBarrier *airThreadBarrierNew(unsigned numUsers);
extern int airThreadBarrierWait(airThreadBarrier *barrier);
extern airThreadBarrier *airThreadBarrierNix(airThreadBarrier *barrier);

/*
******** airFP enum
**
** the different kinds of floating point number afforded by IEEE 754,
** and the values returned by airFPClass_f().
**
** The values probably won't agree with those in #include's like
** ieee.h, ieeefp.h, fp_class.h.  This is because IEEE 754 hasn't
** defined standard values for these, so everyone does it differently.
** 
** This enum uses underscores (against teem convention) to help
** legibility while also conforming to the spirit of the somewhat
** standard naming conventions
*/
enum {
  airFP_Unknown,               /*  0: nobody knows */
  airFP_SNAN,                  /*  1: signalling NaN */
  airFP_QNAN,                  /*  2: quiet NaN */
  airFP_POS_INF,               /*  3: positive infinity */
  airFP_NEG_INF,               /*  4: negative infinity */
  airFP_POS_NORM,              /*  5: positive normalized non-zero */
  airFP_NEG_NORM,              /*  6: negative normalized non-zero */
  airFP_POS_DENORM,            /*  7: positive denormalized non-zero */
  airFP_NEG_DENORM,            /*  8: negative denormalized non-zero */
  airFP_POS_ZERO,              /*  9: +0.0, positive zero */
  airFP_NEG_ZERO,              /* 10: -0.0, negative zero */
  airFP_Last                   /* after the last valid one */
};
/* 754.c: IEEE-754 related stuff values */
typedef union {
  unsigned int i;
  float f;
} airFloat;
typedef union {
  airULLong i;
  double d;
} airDouble;
extern air_export const int airMyQNaNHiBit;
extern air_export const int airMyBigBitField;
extern float airFPPartsToVal_f(int sign, int exp, int frac);
extern void airFPValToParts_f(int *signP, int *expP, int *fracP, float v);
extern double airFPPartsToVal_d(int sign, int exp, airULLong frac);
extern void airFPValToParts_d(int *signP, int *expP, airULLong *fracP,
			      double v);
extern float airFPGen_f(int cls);
extern double airFPGen_d(int cls);
extern int airFPClass_f(float val);
extern int airFPClass_d(double val);
extern void airFPFprintf_f(FILE *file, float val);
extern void airFPFprintf_d(FILE *file, double val);
extern air_export const airFloat airFloatQNaN;
extern air_export const airFloat airFloatSNaN;
extern air_export const airFloat airFloatPosInf;
extern air_export const airFloat airFloatNegInf;
extern float airNaN();
extern int airIsNaN(float f);
extern int airIsInf_f(float f);
extern int airIsInf_d(double d);
extern int airExists_f(float f);
extern int airExists_d(double d);

/*
******** airType
**
** Different types which air cares about.
** Currently only used in the command-line parsing, but perhaps will
** be used elsewhere in air later
*/
enum {
  airTypeUnknown,   /* 0 */
  airTypeBool,      /* 1 */
  airTypeInt,       /* 2 */
  airTypeFloat,     /* 3 */
  airTypeDouble,    /* 4 */
  airTypeChar,      /* 5 */
  airTypeString,    /* 6 */
  airTypeEnum,      /* 7 */
  airTypeOther,     /* 8 */
  airTypeLast
};
#define AIR_TYPE_MAX   8
/* parseAir.c */
extern double airAtod(const char *str);
extern int airSingleSscanf(const char *str, const char *fmt, void *ptr);
extern airEnum *airBool;
extern int airParseStrB(int *out, const char *s,
			const char *ct, int n, ... /* nothing used */);
extern int airParseStrI(int *out, const char *s,
			const char *ct, int n, ... /* nothing used */);
extern int airParseStrF(float *out, const char *s,
			const char *ct, int n, ... /* nothing used */);
extern int airParseStrD(double *out, const char *s,
			const char *ct, int n, ... /* nothing used */);
extern int airParseStrC(char *out, const char *s,
			const char *ct, int n, ... /* nothing used */);
extern int airParseStrS(char **out, const char *s,
			const char *ct, int n, ... /* REQUIRED, even if n>1:
						      int greedy */);
extern int airParseStrE(int *out, const char *s,
			const char *ct, int n, ... /* REQUIRED: airEnum *e */);
extern air_export int (*airParseStr[AIR_TYPE_MAX+1])(void *, const char *,
					  const char *, int, ...);

/* string.c */
extern char *airStrdup(const char *s);
extern size_t airStrlen(const char *s);
extern air_export int airStrtokQuoting;
extern char *airStrtok(char *s, const char *ct, char **last);
extern int airStrntok(const char *s, const char *ct);
extern char *airStrtrans(char *s, char from, char to);
extern int airEndsWith(const char *s, const char *suff);
extern char *airUnescape(char *s);
extern char *airOneLinify(char *s);
extern char *airToLower(char *str);
extern char *airToUpper(char *str);
extern int airOneLine(FILE *file, char *line, int size);

/* sane.c */
/*
******** airInsane enum
** 
** reasons for why airSanity() failed (specifically, the possible
** return values for airSanity()
*/
enum {
  airInsane_not,           /*  0: actually, all sanity checks passed */
  airInsane_endian,        /*  1: airMyEndian is wrong */
  airInsane_pInfExists,    /*  2: AIR_EXISTS(positive infinity) was true */
  airInsane_nInfExists,    /*  3: AIR_EXISTS(negative infinity) was true */
  airInsane_NaNExists,     /*  4: AIR_EXISTS(NaN) was true */
  airInsane_FltDblFPClass, /*  5: double -> float assignment messed up the
			       airFPClass_f() of the value */
  airInsane_QNaNHiBit,     /*  6: airMyQNaNHiBit is wrong */
  airInsane_dio,           /*  7: airMyDio set to something invalid */
  airInsane_32Bit,         /*  8: airMy32Bit is wrong */
  airInsane_FISize,        /*  9: sizeof(float), sizeof(int) not 4 */
  airInsane_DLSize         /* 10: sizeof(double), sizeof(airLLong) not 8 */
};
#define AIR_INSANE_MAX        10
extern const char *airInsaneErr(int insane);
extern int airSanity();

/* miscAir.c */
extern air_export const char *airTeemVersion;
extern air_export const char *airTeemReleaseDate;
extern air_export const char airMyFmt_size_t[];
extern air_export const int airMy32Bit;
extern void airSrand(void);
extern double airRand(void);
extern int airRandInt(int N);
extern void airShuffle(int *buff, int N, int perm);
extern void *airNull(void);
extern void *airSetNull(void **ptrP);
extern void *airFree(void *ptr);
extern void *airFreeP(void *_ptrP);
extern FILE *airFopen(const char *name, FILE *std, const char *mode);
extern FILE *airFclose(FILE *file);
extern char *airDoneStr(float start, float here, float end, char *str);
extern double airTime();
extern double airCbrt(double);
extern double airSgnPow(double, double);
extern int airSgn(double);
extern int airLog2(float n);
extern int airSinglePrintf(FILE *file, char *str, const char *fmt, ...);
extern void airBinaryPrintUInt(FILE *file, int digits, unsigned int N);
extern double airErf(double x);
extern double airGaussian(double x, double mean, double stdv);
extern air_export const char airTypeStr[AIR_TYPE_MAX+1][AIR_STRLEN_SMALL];
extern air_export const int airTypeSize[AIR_TYPE_MAX+1];
extern int airILoad(void *v, int t);
extern float airFLoad(void *v, int t);
extern double airDLoad(void *v, int t);
extern int airIStore(void *v, int t, int i);
extern float airFStore(void *v, int t, float f);
extern double airDStore(void *v, int t, double d);

/* dio.c */
/*
******** airNoDio enum
**
** reasons for why direct I/O won't be used with a particular 
** file/pointer combination
*/
enum {
  airNoDio_okay,    /*  0: actually, you CAN do direct I/O */
  airNoDio_arch,    /*  1: teem thinks this architecture can't do it */
  airNoDio_format,  /*  2: teem thinks given data file format can't use it */
  airNoDio_file,    /*  3: given NULL file */
  airNoDio_std,     /*  4: DIO isn't possible for std{in|out|err} */
  airNoDio_fd,      /*  5: couldn't get underlying file descriptor */
  airNoDio_fcntl,   /*  6: the required fcntl() call failed */
  airNoDio_small,   /*  7: requested size is too small */
  airNoDio_size,    /*  8: requested size not a multiple of d_miniosz */
  airNoDio_ptr,     /*  9: pointer not multiple of d_mem */
  airNoDio_fpos,    /* 10: current file position not multiple of d_miniosz */
  airNoDio_test,    /* 11: couldn't memalign() even a small bit of memory */
  airNoDio_disable  /* 12: someone disabled it with airDisableDio */
};
#define AIR_NODIO_MAX  12
extern const char *airNoDioErr(int noDio);
extern air_export const int airMyDio;
extern air_export int airDisableDio;
extern int airDioInfo(int *mem, int *min, int *max, FILE *file);
extern int airDioTest(size_t size, FILE *file, void *ptr);
extern size_t airDioRead(FILE *file, void **ptrP, size_t size);
extern size_t airDioWrite(FILE *file, void *ptr, size_t size);

/* mop.c: clean-up utilities */
enum {
  airMopNever,
  airMopOnError,
  airMopOnOkay,
  airMopAlways
};
typedef void *(*airMopper)(void *);
typedef struct {
  void *ptr;         /* the thing to be processed */
  airMopper mop;     /* the function to which does the processing */
  int when;          /* from the airMopWhen enum */
} airMop;
extern airArray *airMopNew(void);
extern void airMopAdd(airArray *arr,
		      void *ptr, airMopper mop, int when);
extern void airMopSub(airArray *arr, void *ptr, airMopper mop);
extern void airMopMem(airArray *arr, void *_ptrP, int when);
extern void airMopUnMem(airArray *arr, void *_ptrP);
extern void airMopPrint(airArray *arr, void *_str, int when);
extern void airMopDone(airArray *arr, int error);
extern void airMopError(airArray *arr);
extern void airMopOkay(airArray *arr);
extern void airMopDebug(airArray *arr);

/*******     the interminable sea of defines and macros     *******/

#define AIR_TRUE 1
#define AIR_FALSE 0
#define AIR_WHITESPACE " \t\n\r\v\f"       /* K+R pg. 157 */

/*
******** AIR_ENDIAN, AIR_QNANHIBIT, AIR_DIO, AIR_BIGBITFIELD
**
** These reflect particulars of hardware which we're running on.
** The reason to have these in addition to TEEM_ENDIAN, TEEM_DIO, etc.,
** is that those are not by default defined for every source-file
** compilation: the teem library has to define NEED_ENDIAN, NEED_DIO, etc,
** and these in turn generate appropriate compile command-line flags
** by Common.mk. By having these defined here, they become available
** to anyone who simply links against the air library (and includes air.h),
** with no command-line flags required, and no usage of Common.mk required.
*/
#define AIR_ENDIAN (airMyEndian)
#define AIR_QNANHIBIT (airMyQNaNHiBit)
#define AIR_DIO (airMyDio)
#define AIR_32BIT (airMy32Bit)
#define AIR_BIGBITFIELD (airMyBigBitField)

/*
******** AIR_NAN, AIR_QNAN, AIR_SNAN, AIR_POS_INF, AIR_NEG_INF
**
** its nice to have these values available without the cost of a 
** function call.
**
** NOTE: AIR_POS_INF and AIR_NEG_INF correspond to the _unique_
** bit-patterns which signify positive and negative infinity.  With
** the NaNs, however, they are only one of many possible
** representations.
*/
#define AIR_NAN  (airFloatQNaN.f)
#define AIR_QNAN (airFloatQNaN.f)
#define AIR_SNAN (airFloatSNaN.f)
#define AIR_POS_INF (airFloatPosInf.f)
#define AIR_NEG_INF (airFloatNegInf.f)

/* 
******** AIR_EXISTS
**
** is non-zero (true) only for values which are not NaN or +/-infinity
** 
** You'd think that (x == x) might work, but no no no, some optimizing
** compilers (e.g. SGI's cc) say "well of course they're equal, for all
** possible values".  Bastards!
**
** One of the benefits of IEEE 754 floating point numbers is that
** gradual underflow means that x = y <==> x - y = 0 for any (positive
** or negative) normalized or denormalized float.  Otherwise this
** macro could not be valid; some floating point conventions say that
** a zero-valued exponent means zero, regardless of the mantissa.
**
** However, there MAY be problems on machines which use extended
** (80-bit) floating point registers, such as Intel chips- where the
** same initial value 1) directly read from the register, versus 2)
** saved to memory and loaded back, may end up being different.  I
** have yet to produce this behavior, or convince myself it can't
** happen.  If you have problems, then use the version of the macro
** which is a function call to airExists_d(), and please email me:
** gk@cs.utah.edu
**
** The reason to #define AIR_EXISTS as airExists_d is that on some
** optimizing compilers, the !((x) - (x)) doesn't work.  This has been
** the case on Windows and 64-bit irix6 (64 bit) with -Ofast.  If
** airSanity fails because a special value "exists", then use the
** first version of AIR_EXISTS.
**
** There are two performance consequences of using airExists_d(x):
** 1) Its a function call (but WIN32 can __inline it)
** 2) (via AIR_EXISTS_G) It requires bit-wise operations on 64-bit
** ints, which might be terribly slow.
**
** The reason for using airExists_d and not airExists_f is for
** doubles > FLT_MAX: airExists_f would say these are infinity.
*/
#ifdef _WIN32
#define AIR_EXISTS(x) (airExists_d(x))
#else
#define AIR_EXISTS(x) (!((x) - (x)))
#endif

/*
******** AIR_EXISTS_F(x)
**
** This is another way to check for non-specialness (not NaN, not
** +inf, not -inf) of a _float_, by making sure the exponent field
** isn't all ones.
**
** Unlike !((x) - (x)) or airExists(x), the argument to this macro
** MUST MUST MUST be a float, and the float must be of the standard
** 32-bit size, which must also be the size of an int.  The reason for
** this constraint is that macros are not functions, so there is no
** implicit cast or conversion to a single type.  Casting the address
** of the macro arg to an int* only works when the arg has the same
** size as an int.
**
** No cross-platform comparitive timings have been done to compare the
** speed of !((x) - (x)) versus airExists() versus AIR_EXISTS_F()
** 
** This macro is endian-safe.
*/
#define AIR_EXISTS_F(x) ((*(unsigned int*)&(x) & 0x7f800000) != 0x7f800000)

/*
******** AIR_EXISTS_D(x)
**
** like AIR_EXISTS_F(), but the argument here MUST be a double
*/
#define AIR_EXISTS_D(x) (                               \
  (*(airULLong*)&(x) & AIR_ULLONG(0x7ff0000000000000))  \
    != AIR_ULLONG(0x7ff0000000000000))

/*
******** AIR_ISNAN_F(x)
**
** detects if a float is NaN by looking at the bits, without relying on
** any of its arithmetic properties.  As with AIR_EXISTS_F(), this only
** works when the argument really is a float, and when floats are 4-bytes
*/
#define AIR_ISNAN_F(x) (((*(unsigned int*)&(x) & 0x7f800000)==0x7f800000) && \
			 (*(unsigned int*)&(x) & 0x007fffff))

/*
******** AIR_MAX(a,b), AIR_MIN(a,b), AIR_ABS(a)
**
** the usual
*/
#define AIR_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AIR_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AIR_ABS(a) ((a) > 0 ? (a) : -(a))

/*
******** AIR_COMPARE(a,b)
**
** the sort of compare that qsort() wants for ascending sort
*/
#define AIR_COMPARE(a,b) ((a) < (b)     \
		          ? -1          \
                          : ((a) > (b) \
                             ? 1        \
                             : 0))

/*
******** AIR_FREE, AIR_FCLOSE
**
** just to help you set all pointers to free'd data and fclose'd files to NULL
*/
#define AIR_FREE(x) (x) = airFree((x))
#define AIR_FCLOSE(f) (f) = airFclose((f))


/*
******** AIR_IN_OP(a,b,c), AIR_IN_CL(a,b,c)
**
** is true if the middle argument is in the open/closed interval
** defined by the first and third arguments
** 
** AIR_IN_OP is new name for old AIR_BETWEEN
** AIR_IN_CL is new name for odl AIR_INSIDE
*/
#define AIR_IN_OP(a,b,c) ((a) < (b) && (b) < (c))     /* closed interval */
#define AIR_IN_CL(a,b,c) ((a) <= (b) && (b) <= (c))   /* open interval */

/*
******** AIR_CLAMP(a,b,c)
**
** returns the middle argument, after being clamped to the closed
** interval defined by the first and third arguments
*/
#define AIR_CLAMP(a,b,c) ((b) < (a)        \
		 	   ? (a)           \
			   : ((b) > (c)    \
			      ? (c)        \
			      : (b)))

/*
******** AIR_MOD(i, N)
**
** returns that integer in [0, N-1] which is i plus a multiple of N. It
** may be unfortunate that the expression (i)%(N) appears three times;
** this should be inlined.  Or perhaps the compiler's optimizations
** (common sub-expression elimination) will save us.
**
** Note: integer divisions are not very fast on some modern chips;
** don't go silly using this one.
*/
#define AIR_MOD(i, N) ((i)%(N) >= 0 ? (i)%(N) : N + (i)%(N))

/*
******** AIR_LERP(w, a, b)
**
** returns a when w=0, and b when w=1, and linearly varies in between
*/
#define AIR_LERP(w, a, b) ((w)*((b) - (a)) + (a))

/*
******** AIR_AFFINE(i,x,I,o,O)
**
** given intervals [i,I], [o,O] and a value x which may or may not be
** inside [i,I], return the value y such that y stands in the same
** relationship to [o,O] that x does with [i,I].  Or:
**
**    y - o         x - i     
**   -------   =   -------
**    O - o         I - i
**
** It is the callers responsibility to make sure I-i and O-o are 
** both non-zero.  Strictly speaking, real problems arise only when
** when I-i is zero: division by zero generates either NaN or infinity
*/
#define AIR_AFFINE(i,x,I,o,O) ( \
((double)(O)-(o))*((double)(x)-(i)) / ((double)(I)-(i)) + (o))

/*
******** AIR_DELTA(i,x,I,o,O)
**
** given intervals [i,I] and [o,O], calculates the number y such that
** a change of x within [i,I] is proportional to a change of y within
** [o,O].  Or:
**
**      y             x     
**   -------   =   -------
**    O - o         I - i
**
** It is the callers responsibility to make sure I-i and O-o are 
** both non-zero
*/
#define AIR_DELTA(i,x,I,o,O) ( \
((double)(O)-(o))*((double)(x)) / ((double)(I)-(i)) )

/*
******** AIR_INDEX(i,x,I,L,t)
**
** READ CAREFULLY!!
**
** Utility for mapping a floating point x in given range [i,I] to the
** index of an array with L elements, AND SAVES THE INDEX INTO GIVEN
** VARIABLE t, WHICH MUST BE OF SOME INTEGER TYPE because this relies
** on the implicit cast of an assignment to truncate away the
** fractional part.  ALSO, t must be of a type large enough to hold
** ONE GREATER than L.  So you can't pass a variable of type unsigned
** char if L is 256
**
** DOES NOT DO BOUNDS CHECKING: given an x which is not inside [i,I],
** this may produce an index not inside [0,L-1] (but it won't always
** do so: the output being outside range [0,L-1] is not a reliable
** test of the input being outside range [i, I]).  The mapping is
** accomplished by dividing the range from i to I into L intervals,
** all but the last of which is half-open; the last one is closed.
** For example, the number line from 0 to 3 would be divided as
** follows for a call with i = 0, I = 4, L = 4:
**
** index:       0    1    2    3 = L-1
** intervals: [   )[   )[   )[    ]
**            |----|----|----|----|
** value:     0    1    2    3    4
**
** The main point of the diagram above is to show how I made the
** arbitrary decision to orient the half-open interval, and which
** end has the closed interval.
**
** Note that AIR_INDEX(0,3,4,4,t) and AIR_INDEX(0,4,4,4,t) both set t = 3
**
** The reason that this macro requires a argument for saving the
** result is that this is the easiest way to avoid extra conditionals.
** Otherwise, we'd have to do some check to see if x is close enough
** to I so that the generated index would be L and not L-1.  "Close
** enough" because due to precision problems you can have an x < I
** such that (x-i)/(I-i) == 1, which was a bug with the previous version
** of this macro.  It is far simpler to just do the index generation
** and then do the sneaky check to see if the index is too large by 1.
** We are relying on the fact that C _defines_ boolean true to be exactly 1.
**
** Note also that we are never explicity casting to one kind of int or
** another-- the given t can be any integral type, including long long.
*/
#define AIR_INDEX(i,x,I,L,t) ( \
(t) = (L) * ((double)(x)-(i)) / ((double)(I)-(i)), \
(t) -= ((t) == (L)) )

/*
******** AIR_ROUNDUP, AIR_ROUNDDOWN
**
** rounds integers up or down; just wrappers around floor and ceil
*/
#define AIR_ROUNDUP(x)   ((int)(floor((x)+0.5)))
#define AIR_ROUNDDOWN(x) ((int)(ceil((x)-0.5)))

/*
******** _AIR_SIZE_T_FMT
**
** This is the format string to use when printf/fprintf/sprintf-ing 
** a value of type size_t.  In C99, "%z" serves this purpose.
**
** This is not a useful macro for the world at large- only for teem
** source files.  Why: we need to leave this as a bare string, so that
** we can exploit C's implicit string concatenation in forming a
** format string.  Therefore, unlike the definition of AIR_ENDIAN,
** AIR_DIO, etc, AIR_SIZE_T_FMT can NOT just refer to a const variable
** (like airMyEndian).  Therefore, TEEM_32BIT has to be defined for
** ALL source files which want to use AIR_SIZE_T_FMT, and to be
** conservative, that's all teem files.  The converse is, since there is
** no expectation that other projects which use teem will be defining
** TEEM_32BIT, this is not useful outside teem, thus the leading _.
*/
#ifdef __APPLE__
#  define _AIR_SIZE_T_FMT "%lu"
#else
#  if TEEM_32BIT == 0
#    define _AIR_SIZE_T_FMT "%lu"
#  elif TEEM_32BIT == 1
#    define _AIR_SIZE_T_FMT "%u"
#  else
#    define _AIR_SIZE_T_FMT "(no _AIR_SIZE_T_FMT w/out TEEM_32BIT %*d)"
#  endif
#endif

/*
******** AIR_MEMCPY1, AIR_MEMCPY2, AIR_MEMCPY4, AIR_MEMCPY8, AIR_MEMCPY
**
** THESE ARE NOT GENERALY PURPOSE memcpy() REPLACEMENTS!
**
** They offer a speed-up over memcpy() by eliminating calls to in on
** very small lengths of memory, BUT ONLY when the addresses involved
** are a multiple of the number of bytes to be copied.
** AIR_MEMCPY{1,2,4,8} will (like memcpy) return the first argument, but
** AIR_MEMCPY does not.
**
** Using these macros is, more than other macros, playing with fire!
** You can very very easily get a (fatal) bus error by trying to
** dereference a pointer as say, an int, just because 4 bytes need to
** be copied, even when the pointer has nothing to do with ints, and
** is therefore holding an address which is NOT a multiple of four.
** Hence the warnings above.
*/
#define AIR_MEMCPY1(a, b)      (*((char*)(a)) =      *((char*)(b)))
#define AIR_MEMCPY2(a, b)     (*((short*)(a)) =     *((short*)(b)))
#define AIR_MEMCPY4(a, b)       (*((int*)(a)) =       *((int*)(b)))
#define AIR_MEMCPY8(a, b) (*((long long*)(a)) = *((long long*)(b)))
#define AIR_MEMCPY(a, b, sz)                  \
switch (sz) {                                 \
  case 1: AIR_MEMCPY1(a, b); break;           \
  case 2: AIR_MEMCPY2(a, b); break;           \
  case 4: AIR_MEMCPY4(a, b); break;           \
  case 8: AIR_MEMCPY8(a, b); break;           \
  default: memcpy((a), (b), (sz)); break;     \
}

#ifdef __cplusplus
}
#endif

#endif /* AIR_HAS_BEEN_INCLUDED */
