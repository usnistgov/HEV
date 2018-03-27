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

int
_nrrdFormatText_available(void) {
  
  return AIR_TRUE;
}

int
_nrrdFormatText_nameLooksLike(const char *fname) {
  
  return (airEndsWith(fname, NRRD_EXT_TEXT)
	  || airEndsWith(fname, ".text")
	  || airEndsWith(fname, ".ascii"));
}

int
_nrrdFormatText_fitsInto(const Nrrd *nrrd, const NrrdEncoding *encoding,
			 int useBiff) {
  char me[]="_nrrdFormatText_fitsInto", err[AIR_STRLEN_MED];
  
  /* encoding ignored- always ascii */
  if (!(1  == nrrd->dim || 2 == nrrd->dim)) {
    sprintf(err, "%s: dimension is %d, not 1 or 2", me, nrrd->dim);
    biffMaybeAdd(NRRD, err, useBiff); 
    return AIR_FALSE;
  }
  if (nrrdTypeBlock == nrrd->type) {
    sprintf(err, "%s: can't save blocks to plain text", me);
    biffMaybeAdd(NRRD, err, useBiff); 
    return AIR_FALSE;
  }
  /* NOTE: type of array not guaranteed to survive */
  return AIR_TRUE;
}

int
_nrrdFormatText_contentStartsLike(NrrdIO *nio) {
  float oneFloat;

  return (NRRD_COMMENT_CHAR == nio->line[0] 
	  || airParseStrF(&oneFloat, nio->line, _nrrdTextSep, 1));
}

int
_nrrdFormatText_read(FILE *file, Nrrd *nrrd, NrrdIO *nio) {
  char me[]="_nrrdFormatText_read", err[AIR_STRLEN_MED], *errS;
  const char *fs;
  int line, len, ret, sx, sy, settwo = 0, gotOnePerAxis = AIR_FALSE;
  /* fl: first line, al: all lines */
  airArray *flArr, *alArr;
  float *fl, *al, oneFloat;
  
  if (!_nrrdFormatText_contentStartsLike(nio)) {
    sprintf(err, "%s: this doesn't look like a %s file", me, 
	    nrrdFormatText->name);
    biffAdd(NRRD, err); return 1;
  }

  /* this goofiness is just to leave the nrrd as we found it
     (specifically, nrrd->dim) when we hit an error */
#define UNSETTWO if (settwo) nrrd->dim = settwo

  /* we only get here with the first line already in nio->line */
  line = 1;
  len = strlen(nio->line);
  
  if (0 == nrrd->dim) {
    settwo = nrrd->dim;
    nrrd->dim = 2;
  }
  /* first, we get through comments */
  while (NRRD_COMMENT_CHAR == nio->line[0]) {
    nio->pos = 1;
    nio->pos += strspn(nio->line + nio->pos, _nrrdFieldSep);
    ret = _nrrdReadNrrdParseField(nrrd, nio, AIR_FALSE);
    /* could we parse anything? */
    if (!ret) {
      /* being unable to parse a comment as a nrrd field is not 
	 any kind of error */
      goto plain;
    }
    if (nrrdField_comment == ret) {
      ret = 0;
      goto plain;
    }
    fs = airEnumStr(nrrdField, ret);
    if (!_nrrdFieldValidInText[ret]) {
      if (nrrdStateVerboseIO) {
	fprintf(stderr, "(%s: field \"%s\" not allowed in plain text "
		"--> plain comment)\n", me, fs);
      }
      ret = 0;
      goto plain;
    }
    /* when reading plain text, we simply ignore repetitions of a field */
    if (!nio->seen[ret]
	&& _nrrdReadNrrdParseInfo[ret](nrrd, nio, AIR_TRUE)) {
      errS = biffGetDone(NRRD);
      if (nrrdStateVerboseIO) {
	fprintf(stderr, "%s: %s", me, errS);
	fprintf(stderr, "(%s: malformed field \"%s\" --> plain comment)\n",
		me, fs);
      }
      if (nrrdField_dimension == ret) {
	/* "# dimension: 0" lead nrrd->dim being set to 0 */
	nrrd->dim = 2;
      }
      free(errS);
      ret = 0;
      goto plain;
    }
    if (nrrdField_dimension == ret) {
      if (!(1 == nrrd->dim || 2 == nrrd->dim)) {
	if (nrrdStateVerboseIO) {
	  fprintf(stderr, "(%s: plain text dimension can only be 1 or 2; "
		  "resetting to 2)\n", me);
	}
	nrrd->dim = 2;
      }
      if (1 == nrrd->dim && gotOnePerAxis) {
	fprintf(stderr, "(%s: already parsed per-axis field, can't reset "
		"dimension to 1; resetting to 2)\n", me);
	nrrd->dim = 2;
      }
    }
    if (_nrrdFieldOnePerAxis[ret]) 
      gotOnePerAxis = AIR_TRUE;
    nio->seen[ret] = AIR_TRUE;
  plain:
    if (!ret) {
      if (nrrdCommentAdd(nrrd, nio->line + 1)) {
	sprintf(err, "%s: couldn't add comment", me);
	biffAdd(NRRD, err); UNSETTWO; return 1;
      }
    }
    if (_nrrdOneLine(&len, nio, file)) {
      sprintf(err, "%s: error getting a line", me);
      biffAdd(NRRD, err); UNSETTWO; return 1;
    }
    if (!len) {
      sprintf(err, "%s: hit EOF before any numbers parsed", me);
      biffAdd(NRRD, err); UNSETTWO; return 1;
    }
    line++;
  }

  /* we supposedly have a line of numbers, see how many there are */
  if (!airParseStrF(&oneFloat, nio->line, _nrrdTextSep, 1)) {
    sprintf(err, "%s: couldn't parse a single number on line %d", me, line);
    biffAdd(NRRD, err); UNSETTWO; return 1;
  }
  flArr = airArrayNew((void**)&fl, NULL, sizeof(float), _NRRD_TEXT_INCR);
  if (!flArr) {
    sprintf(err, "%s: couldn't create array for first line values", me);
    biffAdd(NRRD, err); UNSETTWO; return 1;
  }
  for (sx=1; 1; sx++) {
    /* there is obviously a limit to the number of numbers that can 
       be parsed from a single finite line of input text */
    if (airArraySetLen(flArr, sx)) {
      sprintf(err, "%s: couldn't alloc space for %d values", me, sx);
      biffAdd(NRRD, err); UNSETTWO; return 1;
    }
    if (sx > airParseStrF(fl, nio->line, _nrrdTextSep, sx)) {
      /* We asked for sx ints and got less.  We know that we successfully
	 got one value, so we did succeed in parsing sx-1 values */
      sx--;
      break;
    }
  }
  flArr = airArrayNuke(flArr);
  if (1 == nrrd->dim && 1 != sx) {
    sprintf(err, "%s: wanted 1-D nrrd, but got %d values on 1st line", me, sx);
    biffAdd(NRRD, err); UNSETTWO; return 1;
  }
  
  /* now see how many more lines there are */
  alArr = airArrayNew((void**)&al, NULL, sx*sizeof(float), _NRRD_TEXT_INCR);
  if (!alArr) {
    sprintf(err, "%s: couldn't create data buffer", me);
    biffAdd(NRRD, err); UNSETTWO; return 1;
  }
  sy = 0;
  while (len) {
    if (-1 == airArrayIncrLen(alArr, 1)) {
      sprintf(err, "%s: couldn't create scanline of %d values", me, sx);
      biffAdd(NRRD, err); UNSETTWO; return 1;
    }
    ret = airParseStrF(al + sy*sx, nio->line, _nrrdTextSep, sx);
    if (sx > ret) {
      sprintf(err, "%s: could only parse %d values (not %d) on line %d",
	      me, ret, sx, line);
      biffAdd(NRRD, err); UNSETTWO; return 1;
    }
    sy++;
    line++;
    if (_nrrdOneLine(&len, nio, file)) {
      sprintf(err, "%s: error getting a line", me);
      biffAdd(NRRD, err); UNSETTWO; return 1;
    }
  }
  /*
  fprintf(stderr, "%s: nrrd->dim = %d, sx = %d; sy = %d\n",
	  me, nrrd->dim, sx, sy);
  */
  
  switch (nrrd->dim) {
  case 2:
    if (nrrdMaybeAlloc(nrrd, nrrdTypeFloat, 2, sx, sy)) {
      sprintf(err, "%s: couldn't allocate plain text data", me);
      biffAdd(NRRD, err); UNSETTWO; return 1;
    }
    break;
  case 1:
    if (nrrdMaybeAlloc(nrrd, nrrdTypeFloat, 1, sy)) {
      sprintf(err, "%s: couldn't allocate plain text data", me);
      biffAdd(NRRD, err); UNSETTWO; return 1;
    }
    break;
  default:
    fprintf(stderr, "%s: PANIC about to save, but dim = %d\n", me, nrrd->dim);
    exit(1);
    break;
  }
  memcpy(nrrd->data, al, sx*sy*sizeof(float));
  
  alArr = airArrayNuke(alArr);
  return 0;
}

int
_nrrdFormatText_write(FILE *file, const Nrrd *nrrd, NrrdIO *nio) {
  char cmt[AIR_STRLEN_SMALL], buff[AIR_STRLEN_SMALL];
  size_t I;
  int i, x, y, sx, sy;
  void *data;
  float val;

  sprintf(cmt, "%c ", NRRD_COMMENT_CHAR);
  if (!nio->bareText) {
    if (1 == nrrd->dim) {
      _nrrdFprintFieldInfo (file, cmt, nrrd, nio, nrrdField_dimension);
    }
    for (i=1; i<=NRRD_FIELD_MAX; i++) {
      if (_nrrdFieldValidInText[i]
	  && nrrdField_dimension != i  /* dimension is handled above */
	  && _nrrdFieldInteresting(nrrd, nio, i)) {
	_nrrdFprintFieldInfo (file, cmt, nrrd, nio, i);
      }
    }
  }

  if (1 == nrrd->dim) {
    sx = 1;
    sy = nrrd->axis[0].size;
  }
  else {
    sx = nrrd->axis[0].size;
    sy = nrrd->axis[1].size;
  }
  data = nrrd->data;
  I = 0;
  for (y=0; y<sy; y++) {
    for (x=0; x<sx; x++) {
      val = nrrdFLookup[nrrd->type](data, I);
      nrrdSprint[nrrdTypeFloat](buff, &val);
      if (x) fprintf(file, " ");
      fprintf(file, "%s", buff);
      I++;
    }
    fprintf(file, "\n");
  }
  
  return 0;
}

const NrrdFormat
_nrrdFormatText = {
  "text",
  AIR_FALSE,  /* isImage */
  AIR_TRUE,   /* readable */
  AIR_FALSE,  /* usesDIO */
  _nrrdFormatText_available,
  _nrrdFormatText_nameLooksLike,
  _nrrdFormatText_fitsInto,
  _nrrdFormatText_contentStartsLike,
  _nrrdFormatText_read,
  _nrrdFormatText_write
};

const NrrdFormat *const
nrrdFormatText = &_nrrdFormatText;
