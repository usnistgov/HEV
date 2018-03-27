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
_nrrdEncodingRaw_available(void) {

  return AIR_TRUE;
}

int
_nrrdEncodingRaw_read(Nrrd *nrrd, NrrdIO *nio) {
  char me[]="_nrrdEncodingRaw_read", err[AIR_STRLEN_MED];
  size_t num, bsize, size, ret, dio;
  int car;

  if (nio->skipData) {
    return 0;
  }
  num = nrrdElementNumber(nrrd);
  bsize = num * nrrdElementSize(nrrd);
  size = bsize;
  if (num != bsize/nrrdElementSize(nrrd)) {
    fprintf(stderr,
	    "%s: PANIC: \"size_t\" can't represent byte-size of data.\n", me);
    exit(1);
  }

  if (nio->format->usesDIO) {
    dio = airDioTest(size, nio->dataFile, NULL);
  } else {
    dio = airNoDio_format;
  }
  if (airNoDio_okay == dio) {
    if (nio->format->usesDIO) {
      if (3 <= nrrdStateVerboseIO) {
	fprintf(stderr, "with direct I/O ");
      }
      if (2 <= nrrdStateVerboseIO) {
	fprintf(stderr, "... ");
	fflush(stderr);
      }
    }
    /* airDioRead includes the memory allocation */
    ret = airDioRead(nio->dataFile, &(nrrd->data), size);
    if (size != ret) {
      sprintf(err, "%s: airDioRead() failed", me);
      biffAdd(NRRD, err); return 1;
    }
  } else {
    if (_nrrdCalloc(nrrd)) {
      sprintf(err, "%s: couldn't allocate sufficient memory for all data", me);
      biffAdd(NRRD, err); return 1;
    }
    if (AIR_DIO && nio->format->usesDIO) {
      if (3 <= nrrdStateVerboseIO) {
	fprintf(stderr, "with fread()");
	if (4 <= nrrdStateVerboseIO) {
	  fprintf(stderr, " (why no DIO: %s)", airNoDioErr(dio));
	}
      }
      if (2 <= nrrdStateVerboseIO) {
	fprintf(stderr, " ... ");
	fflush(stderr);
      }
    }
    ret = fread(nrrd->data, nrrdElementSize(nrrd), num, nio->dataFile);
    if (ret != num) {
      sprintf(err, "%s: fread() got only " _AIR_SIZE_T_FMT " %d-byte things, "
	      "not " _AIR_SIZE_T_FMT ,
	      me, ret, nrrdElementSize(nrrd), num);
      biffAdd(NRRD, err); return 1;
    }
  }
  car = fgetc(nio->dataFile);
  if (EOF != car) {
    fprintf(stderr, "%s: WARNING: finished reading raw data, "
	    "but file not at EOF\n", me);
    ungetc(car, nio->dataFile);
  }

  return 0;
}

int
_nrrdEncodingRaw_write(const Nrrd *nrrd, NrrdIO *nio) {
  char me[]="_nrrdEncodingRaw_write", err[AIR_STRLEN_MED];
  size_t size, ret, dio;
  
  if (nio->skipData) {
    return 0;
  }
  /* this shouldn't actually be necessary ... */
  if (!nrrdElementSize(nrrd)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }
  size = nrrdElementNumber(nrrd) * nrrdElementSize(nrrd);
  if (nrrdElementNumber(nrrd) != size/nrrdElementSize(nrrd)) {
    sprintf(err, "%s: \"size_t\" can't represent byte-size of data.", me);
    biffAdd(NRRD, err); return 1;
  }

  if (nio->format->usesDIO) {
    dio = airDioTest(size, nio->dataFile, nrrd->data);
  } else {
    dio = airNoDio_format;
  }
  if (airNoDio_okay == dio) {
    if (nio->format->usesDIO) {
      if (3 <= nrrdStateVerboseIO) {
	fprintf(stderr, "with direct I/O ");
      }
      if (2 <= nrrdStateVerboseIO) {
	fprintf(stderr, "... ");
	fflush(stderr);
      }
    }
    ret = airDioWrite(nio->dataFile, nrrd->data, size);
    if (size != ret) {
      sprintf(err, "%s: airDioWrite failed", me);
      biffAdd(NRRD, err); return 1;
    }
  } else {
    if (AIR_DIO && nio->format->usesDIO) {
      if (3 <= nrrdStateVerboseIO) {
	fprintf(stderr, "with fwrite()");
	if (4 <= nrrdStateVerboseIO) {
	  fprintf(stderr, " (why no DIO: %s)", airNoDioErr(dio));
	}
      }
      if (2 <= nrrdStateVerboseIO) {
	fprintf(stderr, " ... ");
	fflush(stderr);
      }
    }
    ret = fwrite(nrrd->data, nrrdElementSize(nrrd),
		 nrrdElementNumber(nrrd), nio->dataFile);
    if (ret != nrrdElementNumber(nrrd)) {
      sprintf(err, "%s: fwrite() wrote only " _AIR_SIZE_T_FMT 
	      " %d-byte things, not " _AIR_SIZE_T_FMT ,
	      me, ret, nrrdElementSize(nrrd), nrrdElementNumber(nrrd));
      biffAdd(NRRD, err); return 1;
    }
    fflush(nio->dataFile);
    /*
    if (ferror(nio->dataFile)) {
      sprintf(err, "%s: ferror returned non-zero", me);
      biffAdd(NRRD, err); return 1;
    }
    */
  }
  return 0;
}

const NrrdEncoding
_nrrdEncodingRaw = {
  "raw",      /* name */
  "raw",      /* suffix */
  AIR_TRUE,   /* endianMatters */
  AIR_FALSE,  /* isCompression */
  _nrrdEncodingRaw_available,
  _nrrdEncodingRaw_read,
  _nrrdEncodingRaw_write
};

const NrrdEncoding *const
nrrdEncodingRaw = &_nrrdEncodingRaw;
