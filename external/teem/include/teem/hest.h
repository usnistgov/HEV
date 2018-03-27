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

#ifndef HEST_HAS_BEEN_INCLUDED
#define HEST_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include <teem/air.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define hest_export __declspec(dllimport)
#else
#define hest_export
#endif

/*
******** hestCB struct
**
** for when the thing you want to parse from the command-line is not a
** simple boolean, number, airEnum, or string.  hestParse() will not
** allocate anything to store individual things, though it may
** allocate an array in the case of a multiple variable parameter
** option.  If your things are actually pointers to things, then you
** do the allocation in the parse() callback.  In this case, you set
** destroy() to be your "destructor", and it will be called on the
** result of derefencing the argument to parse().
*/
typedef struct {
  size_t size;          /* sizeof() one thing */
  char *type;           /* used by hestGlossary() to describe the type */
  int (*parse)(void *ptr, char *str, char err[AIR_STRLEN_HUGE]);
                        /* how to parse one thing from a string.  This will
			   be called multiple times for multiple parameter
			   options.  A non-zero return value is considered
			   an error.  Error message go in the err string */
  void *(*destroy)(void *ptr);
                        /* if non-NULL, this is the destructor that will be
			   called by hestParseFree() (or by hestParse() if
			   there is an error midway through parsing).  The
			   argument is NOT the same as passed to parse():
			   it is the result of dereferencing the argument
			   to parse() */
} hestCB;

/*
******** hestOpt struct
**
** information which specifies one command-line option
*/
typedef struct {
  char *flag,           /* how the option is identified on the cmd line */
    *name;              /* simple description of option's parameter(s) */
  int type,             /* type of option (from airType enum) */
    min, max;           /* min and max # of parameters for option */
  void *valueP;         /* storage of parsed values */
  char *dflt,           /* default value written out as string */
    *info;              /* description to be printed with "glossary" info */
  int *sawP;            /* used ONLY for multiple variable parameter options
			   (min < max > 2): storage of # of parsed values */
  airEnum *enm;         /* used ONLY for airTypeEnum options */
  hestCB *CB;           /* used ONLY for airTypeOther options */

  /* --------------------- end of user-defined fields */

  int kind,             /* what kind of option is this, based on min and max,
			   set by hestParse() (actually _hestPanic()),
			   later used by hestFree():
			   1: min == max == 0
			      stand-alone flag; no parameters
			   2: min == max == 1
                              single fixed parameter
			   3: min == max >= 2
                              multiple fixed parameters
			   4: min == 0; max == 1;
                              single variable parameter
			   5: max - min >= 1; max >= 2 
                              multiple variable parameters */
    alloc;              /* information about whether flag is non-NULL, and what
			   parameters were used, that determines whether or
			   not memory was allocated by hestParse(); info
			   later used by hestParseFree():
			   0: no free()ing needed
			   1: free(*valueP), either because it is a single
			      string, or because was a dynamically allocated
			      array of non-strings
			   2: free((*valueP)[i]), because they are elements
			      of a fixed-length array of strings
			   3: free((*valueP)[i]) and free(*valueP), because
 			      it is a dynamically allocated array of strings */
} hestOpt;

/*
******** hestParm struct
**
** parameters to control behavior of hest functions. 
**
** GK: Don't even think about storing per-parse state in here.
*/
typedef struct {
  int verbosity,        /* verbose diagnostic messages to stdout */
    respFileEnable,     /* whether or not to use response files */
    elideSingleEnumType,  /* if type is airTypeEnum, and if its a single fixed
			     parameter option, then don't bother printing the
			     type information as part of hestGlossary() */
    elideSingleOtherType, /* like above, but for airTypeOther */
    elideSingleOtherDefault, /* don't display default for single fixed
				airTypeOther parameter */
    elideSingleNonExistFloatDefault, /* if default for a single fixed floating
					point (float or double) parameter
					doesn't AIR_EXIST, then don't display
					the default */
    elideMultipleNonExistFloatDefault,
    elideSingleEmptyStringDefault, /* if default for a single string is empty
				      (""), then don't display default */
    elideMultipleEmptyStringDefault,
    greedySingleString, /* when parsing a single string, whether or not
			   to be greedy (as per airParseStrS) */
    cleverPluralizeOtherY, /* when printing the type for airTypeOther, when
			      the min number of items is > 1, and the type
			      string ends with "y", then pluralize with 
			      "ies" instead of "ys" */
    columns;            /* number of printable columns in output */
  char respFileFlag,    /* the character at the beginning of an argument
			   indicating that this is a response file name */
    respFileComment,    /* comment character for the repose files */
    varParamStopFlag,   /* prefixed by '-' to form the flag which signals
			   the end of a flagged variable parameter option
			   (single or multiple) */
    multiFlagSep;       /* character in flag which signifies that there is
			   a long and short version, and which seperates
			   the two.  Or, can be set to '\0' to disable this
			   behavior entirely. */
} hestParm;

/* defaultsHest.c */
extern hest_export int hestVerbosity;
extern hest_export int hestRespFileEnable;
extern hest_export int hestElideSingleEnumType;
extern hest_export int hestElideSingleOtherType;
extern hest_export int hestElideSingleOtherDefault;
extern hest_export int hestElideSingleNonExistFloatDefault;
extern hest_export int hestElideMultipleNonExistFloatDefault;
extern hest_export int hestElideSingleEmptyStringDefault;
extern hest_export int hestElideMultipleEmptyStringDefault;
extern hest_export int hestGreedySingleString;
extern hest_export int hestCleverPluralizeOtherY;
extern hest_export int hestColumns;
extern hest_export char hestRespFileFlag;
extern hest_export char hestRespFileComment;
extern hest_export char hestVarParamStopFlag;
extern hest_export char hestMultiFlagSep;

/* methodsHest.c */
extern hestParm *hestParmNew(void);
extern hestParm *hestParmFree(hestParm *parm);
extern void hestOptAdd(hestOpt **optP, 
		       char *flag, char *name,
		       int type, int min, int max,
		       void *valueP, const char *dflt, const char *info,
		       ... /* int *sawP, airEnum *enm , hestCB *CB */);
extern hestOpt *hestOptFree(hestOpt *opt);
extern int hestOptCheck(hestOpt *opt, char **errP);

/* parseHest.c */
extern int hestParse(hestOpt *opt, int argc, char **argv,
		     char **errP, hestParm *parm);
extern void *hestParseFree(hestOpt *opt);
extern void hestParseOrDie(hestOpt *opt, int argc, char **argv, hestParm *parm,
			   char *me, char *info,
			   int doInfo, int doUsage, int doGlossary);

/* usage.c */
extern void _hestPrintStr(FILE *f, int indent, int already, int width,
			  const char *_str, int bslash);
extern int hestMinNumArgs(hestOpt *opt);
extern void hestUsage(FILE *file, hestOpt *opt, const char *argv0,
		      hestParm *parm);
extern void hestGlossary(FILE *file, hestOpt *opt, hestParm *parm);
extern void hestInfo(FILE *file, const char *argv0, const char *info,
		     hestParm *parm);

#ifdef __cplusplus
}
#endif

#endif /* HEST_HAS_BEEN_INCLUDED */
