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
** what a NrrdEncoding can assume:
** -- the given nrrd struct has been filled out so that functions
**    like nrrdElementNumber and nrrdElementSize will work correctly.
**
** what a NrrdEncoding has to do:
** -- allocate nrrd->data for the amount of space required (very likely
**    via _nrrdCalloc).  That this has to be done here is because of the
**    restrictions imposed by DirectIO (used by nrrdEncodingRaw).
** -- do nothing on read/write if nio->skipData
** -- read data from nio->dataFile into nrrd->data, or vice versa.
** -- respect nrrdStateVerboseIO with messages to stderr, if possible
** -- in case of error, put text error messages into biff via
**    biffAdd(NRRD, <error char*>)
**
** The "unknown" encoding below is intended to serve as a template for 
** any new encodings being developed.
*/

int
_nrrdEncodingUnknown_available(void) {

  /* insert code here */

  return AIR_FALSE;
}

int
_nrrdEncodingUnknown_read(Nrrd *nrrd, NrrdIO *nio) {
  char me[]="_nrrdEncodingUnknown_read", err[AIR_STRLEN_MED];

  if (nio->skipData) {
    return 0;
  }

  /* insert code here, and remove error handling below */

  sprintf(err, "%s: ERROR!!! trying to read unknown encoding", me);
  biffAdd(NRRD, err);
  return 1;
}

int
_nrrdEncodingUnknown_write(const Nrrd *nrrd, NrrdIO *nio) {
  char me[]="_nrrdEncodingUnknown_write", err[AIR_STRLEN_MED];

  if (nio->skipData) {
    return 0;
  }

  /* insert code here, and remove error handling below */

  sprintf(err, "%s: ERROR!!! trying to write unknown encoding", me);
  biffAdd(NRRD, err);
  return 1;
}

const NrrdEncoding
_nrrdEncodingUnknown = {
  "unknown",  /* name */
  "unknown",  /* suffix */
  AIR_FALSE,  /* endianMatters */
  AIR_FALSE,  /* isCompression */
  _nrrdEncodingUnknown_available,
  _nrrdEncodingUnknown_read,
  _nrrdEncodingUnknown_write
};

const NrrdEncoding *const
nrrdEncodingUnknown = &_nrrdEncodingUnknown;

const NrrdEncoding *const
nrrdEncodingArray[NRRD_ENCODING_TYPE_MAX+1] = {
  &_nrrdEncodingUnknown,
  &_nrrdEncodingRaw,
  &_nrrdEncodingAscii,
  &_nrrdEncodingHex,
  &_nrrdEncodingGzip,
  &_nrrdEncodingBzip2
};

