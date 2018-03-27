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

/*
******** nrrdKeyValueSize
**
** returns the number of key/value pairs in a nrrd
*/
int
nrrdKeyValueSize(const Nrrd *nrrd) {
  
  if (!nrrd) {
    return -1;
  }
  return nrrd->kvpArr->len;
}

/*
******** nrrdKeyValueIndex
**
** given an int in [0 .. #key/value pairs - 1], sets *keyP and *valueP
** to put to the corresponding key and value.
**
** NOTE: like nrrdKeyValueGet, these results point to strings 
** INTERNAL to the nrrd struct.  DO NOT FREE THEM.
*/
void
nrrdKeyValueIndex(const Nrrd *nrrd, char **keyP, char **valueP, int ki) {
  
  if (!( nrrd && keyP && valueP 
	 && AIR_IN_CL(0, ki, nrrd->kvpArr->len-1) )) {
    if (keyP) *keyP = NULL;
    if (valueP) *valueP = NULL;
    return;
  }
  *keyP = nrrd->kvp[0 + 2*ki];
  *valueP = nrrd->kvp[1 + 2*ki];
  return;
}

int
_nrrdKeyValueIdxFind(const Nrrd *nrrd, const char *key) {
  int nk, ki;

  nk = nrrd->kvpArr->len;
  for (ki=0; ki<nk; ki++) {
    if (!strcmp(nrrd->kvp[0 + 2*ki], key)) {
      break;
    }
  }
  return (ki<nk ? ki : -1);
}

void
nrrdKeyValueClear(Nrrd *nrrd) {
  int nk, ki;

  if (!nrrd)
    return;

  nk = nrrd->kvpArr->len;
  for (ki=0; ki<nk; ki++) {
    AIR_FREE(nrrd->kvp[0 + 2*ki]);
    AIR_FREE(nrrd->kvp[1 + 2*ki]);
  }
  airArraySetLen(nrrd->kvpArr, 0);
  
  return;
}

int
nrrdKeyValueErase(Nrrd *nrrd, const char *key) {
  int nk, ki;
  
  if (!( nrrd && key )) {
    /* got NULL pointer */
    return 1;
  }
  ki = _nrrdKeyValueIdxFind(nrrd, key);
  if (-1 == ki) {
    return 0;
  }
  AIR_FREE(nrrd->kvp[0 + 2*ki]);
  AIR_FREE(nrrd->kvp[1 + 2*ki]);
  nk = nrrd->kvpArr->len;
  for (; ki<nk-1; ki++) {
    nrrd->kvp[0 + 2*ki] = nrrd->kvp[0 + 2*(ki+1)];
    nrrd->kvp[1 + 2*ki] = nrrd->kvp[1 + 2*(ki+1)];
  }
  airArrayIncrLen(nrrd->kvpArr, -1);

  return 0;
}

/*
******** nrrdKeyValueAdd
**
** This will COPY the given strings, and so does not depend on
** them existing past the return of this function
*/
int
nrrdKeyValueAdd(Nrrd *nrrd, const char *key, const char *value) {
  int ki;

  if (!( nrrd && key && value )) {
    /* got NULL pointer */
    return 1;
  }
  if (!strlen(key)) {
    /* reject empty keys */
    return 1;
  }
  if (-1 != (ki = _nrrdKeyValueIdxFind(nrrd, key))) {
    AIR_FREE(nrrd->kvp[1 + 2*ki]);
    nrrd->kvp[1 + 2*ki] = airStrdup(value);
  } else {
    ki = airArrayIncrLen(nrrd->kvpArr, 1);
    nrrd->kvp[0 + 2*ki] = airStrdup(key);
    nrrd->kvp[1 + 2*ki] = airStrdup(value);
  }

  return 0;
}

/*
******** nrrdKeyValueGet
**
** This will return a pointer to a string INSIDE the given
** nrrd struct; DO NOT FREE IT.  This is for the sake of
** convenience, not perfect safety (obviously)
*/
char *
nrrdKeyValueGet(const Nrrd *nrrd, const char *key) {
  char *ret;
  int ki;
  
  if (!( nrrd && key )) {
    /* got NULL pointer */
    return NULL;
  }
  if (-1 != (ki = _nrrdKeyValueIdxFind(nrrd, key))) {
    ret = nrrd->kvp[1 + 2*ki];
  } else {
    ret = NULL;
  }
  return ret;
}

void
_nrrdFwriteEscaped(FILE *file, const char *str) {
  int ci;

  for (ci=0; ci<strlen(str); ci++) {
    switch(str[ci]) {
    case '\n':
      fprintf(file, "\\n");
      break;
    case '\\':
      fprintf(file, "\\\\");
      break;
    default:
      fputc(str[ci], file);
      break;
    }
  }
  return;
}

/*
** _nrrdKeyValueFwrite
**
** writes a given key and value to a file, starting with the given
** prefix (if non-NULL), and ending with "\n"
*/
int
_nrrdKeyValueFwrite(FILE *file, const char *prefix, 
		    const char *key, const char *value) {
  
  if (!( file && key && value )) {
    return 1;
  }
  if (prefix) {
    fprintf(file, "%s", prefix);
  }
  _nrrdFwriteEscaped(file, key);
  fprintf(file, ":=");
  _nrrdFwriteEscaped(file, value);
  fprintf(file, "\n");
  return 0;
}
