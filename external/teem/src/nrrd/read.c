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


#include "nrrd.h"
#include "privateNrrd.h"

#if TEEM_BZIP2
#include <bzlib.h>
#endif

#include <teem32bit.h>

char _nrrdRelativePathFlag[] = "./";
char _nrrdFieldSep[] = " \t";
char _nrrdTextSep[] = " ,\t";

/*
** _nrrdOneLine
**
** wrapper around airOneLine; does re-allocation of line buffer
** ("line") in the NrrdIO if needed.  The return value semantics
** are similar, except that what airOneLine would return, we put
** in *lenP.  If there is an error (airOneLine returned -1, 
** something couldn't be allocated), *lenP is set to 0, and 
** we return 1.  HITTING EOF IS NOT ACTUALLY AN ERROR, see code
** below.  Otherwise we return 0.
**
** Does use biff
*/
int
_nrrdOneLine (int *lenP, NrrdIO *nio, FILE *file) {
  char me[]="_nrrdOneLine", err[AIR_STRLEN_MED], **line;
  airArray *lineArr;
  int len, lineIdx;

  if (!( lenP && nio && file)) {
    sprintf(err, "%s: got NULL pointer (%p, %p, %p)", me,
	    lenP, nio, file);
    biffAdd(NRRD, err); return 1;
  }
  if (0 == nio->lineLen) {
    /* nio->line hasn't been allocated for anything */
    nio->line = malloc(3);
    nio->lineLen = 3;
  }
  len = airOneLine(file, nio->line, nio->lineLen);
  if (len <= nio->lineLen) {
    if (-1 == len) {
      sprintf(err, "%s: invalid args to airOneLine()", me);
      biffAdd(NRRD, err); *lenP = 0; return 1;
    }
    /* otherwise we hit EOF before a newline, or the line (possibly empty)
       fit within the nio->line, neither of which is an error here */
    *lenP = len;
    return 0;
  }
  /* else line didn't fit in buffer, so we have to increase line
     buffer size and put the line together in pieces */
  lineArr = airArrayNew((void**)(&line), NULL, sizeof(char *), 1);
  if (!lineArr) {
    sprintf(err, "%s: couldn't allocate airArray", me);
    biffAdd(NRRD, err); *lenP = 0; return 1;
  }
  airArrayPointerCB(lineArr, airNull, airFree);
  while (len == nio->lineLen+1) {
    lineIdx = airArrayIncrLen(lineArr, 1);
    if (-1 == lineIdx) {
      sprintf(err, "%s: couldn't increment line buffer array", me);
      biffAdd(NRRD, err); *lenP = 0; return 1;
    }
    line[lineIdx] = nio->line;
    nio->lineLen *= 2;
    nio->line = (char*)malloc(nio->lineLen);
    if (!nio->line) {
      sprintf(err, "%s: couldn't alloc %d-char line\n", me, nio->lineLen);
      biffAdd(NRRD, err); *lenP = 0; return 1;
    }
    len = airOneLine(file, nio->line, nio->lineLen);
  }
  /* last part did fit in nio->line buffer, also save this into line[] */
  lineIdx = airArrayIncrLen(lineArr, 1);
  if (-1 == lineIdx) {
    sprintf(err, "%s: couldn't increment line buffer array", me);
    biffAdd(NRRD, err); *lenP = 0; return 1;
  }
  line[lineIdx] = nio->line;
  nio->lineLen *= 3;  /* for good measure */
  nio->line = (char*)malloc(nio->lineLen);
  if (!nio->line) {
    sprintf(err, "%s: couldn't alloc %d-char line\n", me, nio->lineLen);
    biffAdd(NRRD, err); *lenP = 0; return 1;
  }
  /* now concatenate everything into a new nio->line */
  strcpy(nio->line, "");
  for (lineIdx=0; lineIdx<lineArr->len; lineIdx++) {
    strcat(nio->line, line[lineIdx]);
  }
  airArrayNuke(lineArr);
  *lenP = strlen(nio->line) + 1;
  return 0;
}

/*
** _nrrdCalloc()
**
** allocates the data for the array.  Only to be called by data readers,
** since it assume the validity of size information, as enforced by
** _nrrdHeaderCheck().
*/
int
_nrrdCalloc (Nrrd *nrrd) {
  char me[]="_nrrdCalloc", err[AIR_STRLEN_MED];

  AIR_FREE(nrrd->data);
  nrrd->data = calloc(nrrdElementNumber(nrrd), nrrdElementSize(nrrd));
  if (!nrrd->data) {
    sprintf(err, "%s: couldn't calloc(" _AIR_SIZE_T_FMT
	    ", %d)", me, nrrdElementNumber(nrrd), nrrdElementSize(nrrd));
    biffAdd(NRRD, err); return 1;
  }
  return 0;
}

/*
******** nrrdLineSkip, nrrdByteSkip
**
** public for the sake of things like "unu make"
*/
int
nrrdLineSkip (NrrdIO *nio) {
  int i, skipRet;
  char me[]="nrrdLineSkip", err[AIR_STRLEN_MED];

  /* For compressed data: If you don't actually have ascii headers on
     top of your gzipped data then you will potentially huge lines
     while _nrrdOneLine looks for line terminations.  Quoting Gordon:
     "Garbage in, Garbage out." */
  
  for (i=1; i<=nio->lineSkip; i++) {
    if (_nrrdOneLine(&skipRet, nio, nio->dataFile)) {
      sprintf(err, "%s: error skipping line %d of %d", me, i, nio->lineSkip);
      biffAdd(NRRD, err); return 1;
    }
    if (!skipRet) {
      sprintf(err, "%s: hit EOF skipping line %d of %d", me, i, nio->lineSkip);
      biffAdd(NRRD, err); return 1;
    }
  }
  return 0;
}

int
nrrdByteSkip (Nrrd *nrrd, NrrdIO *nio) {
  int i, skipRet;
  char me[]="nrrdByteSkip", err[AIR_STRLEN_MED];
  size_t numbytes;

  if (!( nrrd && nio )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (-1 == nio->byteSkip) {
    if (nrrdEncodingRaw != nio->encoding) {
      sprintf(err, "%s: can't backwards byte skipping in %s encoding",
	      me, nio->encoding->name);
      biffAdd(NRRD, err); return 1;
    }
    if (stdin == nio->dataFile) {
      sprintf(err, "%s: can't fseek on stdin", me);
      biffAdd(NRRD, err); return 1;
    }
    numbytes = nrrdElementNumber(nrrd)*nrrdElementSize(nrrd);
    if (fseek(nio->dataFile, -numbytes, SEEK_END)) {
      sprintf(err, "%s: failed to fseek(dataFile, " _AIR_SIZE_T_FMT
	      ", SEEK_END)", me, numbytes);
      biffAdd(NRRD, err); return 1;      
    }
  } else {
    for (i=1; i<=nio->byteSkip; i++) {
      skipRet = fgetc(nio->dataFile);
      if (EOF == skipRet) {
	sprintf(err, "%s: hit EOF skipping byte %d of %d",
		me, i, nio->byteSkip);
	biffAdd(NRRD, err); return 1;
      }
    }
  }
  return 0;
}

/*
******** nrrdRead()
**
** read in nrrd from a given file.  The main job of this function is to
** start reading the file, to determine the format, and then call the
** appropriate format's reader.  This means that the various encoding
** (data) readers can assume that nio->format is usefully set.
**
** The only input information that nio is used for is nio->path, so that
** detached header-relative data files can be found.  
**
*/
int
nrrdRead (Nrrd *nrrd, FILE *file, NrrdIO *nio) {
  char err[AIR_STRLEN_MED], me[] = "nrrdRead";
  int len, fi;
  airArray *mop;

  if (!(file && nrrd)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  mop = airMopNew();
  if (!nio) {
    nio = nrrdIONew();
    if (!nio) {
      sprintf(err, "%s: couldn't alloc I/O struct", me);
      biffAdd(NRRD, err); return 1;
    }
    airMopAdd(mop, nio, (airMopper)nrrdIONix, airMopAlways);
  }

  /* clear out anything in the given nrrd */
  nrrdInit(nrrd);

  if (_nrrdOneLine(&len, nio, file)) {
    sprintf(err, "%s: error getting first line (containing \"magic\")", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  if (!len) {
     sprintf(err, "%s: immediately hit EOF", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  
  nio->format = nrrdFormatUnknown;
  for (fi = nrrdFormatTypeUnknown+1; 
       fi < nrrdFormatTypeLast; 
       fi++) {
    if (nrrdFormatArray[fi]->contentStartsLike(nio)) {
      nio->format = nrrdFormatArray[fi];
      break;
    }
  }
  if (nrrdFormatUnknown == nio->format) {
    sprintf(err, "%s: couldn't parse \"%s\" as magic or beginning of "
	    "any recognized format", me, nio->line);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  
  /* try to read the file */
  if (nio->format->read(file, nrrd, nio)) {
    sprintf(err, "%s: trouble reading %s file", me, nio->format->name);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }

  /* reshape up grayscale images, if desired */
  if (nio->format->isImage && 2 == nrrd->dim && nrrdStateGrayscaleImage3D) {
    if (nrrdAxesInsert(nrrd, nrrd, 0)) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}

/*
** _nrrdSplitName()
**
** splits a file name into a path and a base filename.  The directory
** seperator is assumed to be '/'.  The division between the path
** and the base is the last '/' in the file name.  The path is
** everything prior to this, and base is everything after (so the
** base does NOT start with '/').  If there is not a '/' in the name,
** or if a '/' appears as the last character, then the path is set to
** ".", and the name is copied into base.
*/
void
_nrrdSplitName (char **dirP, char **baseP, const char *name) {
  int i;
  
  if (dirP) {
    AIR_FREE(*dirP);
  }
  if (baseP) {
    AIR_FREE(*baseP);
  }
  i = strrchr(name, '/') - name;
  /* we found a valid break if the last directory character
     is somewhere in the string except the last character */
  if (i>=0 && i<strlen(name)-1) {
    if (dirP) {
      *dirP = airStrdup(name);
      (*dirP)[i] = 0;
    }
    if (baseP) {
      *baseP = airStrdup(name + i + 1);
    }
  } else {
    /* if the name had no slash, its in the current directory, which
       means that we need to explicitly store "." as the header
       directory in case we have header-relative data. */
    if (dirP) {
      *dirP = airStrdup(".");
    }
    if (baseP) {
      *baseP = airStrdup(name);
    }
  }
  return;
}

/*
******** nrrdLoad()
**
** (documentation here)
**
** sneakiness: returns 2 if the reason for problem was a failed fopen().
** 
*/
int
nrrdLoad (Nrrd *nrrd, const char *filename, NrrdIO *nio) {
  char me[]="nrrdLoad", err[AIR_STRLEN_MED];
  FILE *file;
  airArray *mop;


  if (!(nrrd && filename)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  mop = airMopNew();
  if (!nio) {
    nio = nrrdIONew();
    if (!nio) {
      sprintf(err, "%s: couldn't alloc I/O struct", me);
      biffAdd(NRRD, err); return 1;
    }
    airMopAdd(mop, nio, (airMopper)nrrdIONix, airMopAlways);
  }
  
  /* we save the directory of the filename given to us so that if it turns
     out that this is a detached header with a header-relative data file,
     then we will know how to find the data file */
  _nrrdSplitName(&(nio->path), NULL, filename);
  /* printf("!%s: |%s|%s|\n", me, nio->dir, nio->base); */

  if (!( file = airFopen(filename, stdin, "rb") )) {
    sprintf(err, "%s: fopen(\"%s\",\"rb\") failed: %s", 
	    me, filename, strerror(errno));
    biffAdd(NRRD, err); airMopError(mop); return 2;
  }
  airMopAdd(mop, file, (airMopper)airFclose, airMopOnError);
  /* non-error exiting is handled below */

  if (nrrdRead(nrrd, file, nio)) {
    sprintf(err, "%s: trouble reading \"%s\"", me, filename);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  
  if (nrrdFormatNRRD == nio->format
      && nio->keepNrrdDataFileOpen && file == nio->dataFile ) {
    /* we have to keep the datafile open.  If was attached, we can't
       close file, because that is the datafile.  If was detached,
       file != nio->dataFile, so we can close file.  */
  } else {
    /* always close non-NRRD files */
    airFclose(file);
  }

  airMopOkay(mop);
  return 0;
}


