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
** what a NrrdFormat can assume:
** -- that nio->format has been set to you already
** -- for read(): that nio->path has been set to the path of the file being
**    read in, if the information was ever available
** -- for contentStartsLike() and read(): that nio->line contains the
**    first line of of the file, in order to determine the file type
**
** what a NrrdFormat has to do:
** -- respect nio->skipData to whatever extent makes sense on top of how the
**    NrrdEncoding respects it (by making read and write no-ops).  
**    nrrdFormatNRRD, for instance, won't create empty detached data files
**    if nio->skipData.
** -- determing what the NrrdEncoding to use, if there's a choice
** -- respect nrrdStateVerboseIO with messages to stderr, if possible
**
** The "unknown" format is intended as a template for writing new formats.
*/

int
_nrrdFormatUnknown_available(void) {

  /* insert code here */

  return AIR_FALSE;
}

int
_nrrdFormatUnknown_nameLooksLike(const char *filename) {
  
  /* insert code here */

  return AIR_FALSE;
}

int
_nrrdFormatUnknown_fitsInto(const Nrrd *nrrd, const NrrdEncoding *encoding,
			    int useBiff) {
  char me[]="_nrrdFormatUnknown_fitsInto", err[AIR_STRLEN_MED];
  
  if (!(nrrd && encoding)) {
    sprintf(err, "%s: got NULL nrrd (%p) or encoding (%p)",
	    me, nrrd, encoding);
    biffMaybeAdd(NRRD, err, useBiff); 
    return AIR_FALSE;
  }

  /* insert code here */

  return AIR_FALSE;
}

int
_nrrdFormatUnknown_contentStartsLike(NrrdIO *nio) {
  
  /* insert code here */

  return AIR_FALSE;
}

int
_nrrdFormatUnknown_read(FILE *file, Nrrd *nrrd, NrrdIO *nio) {
  char me[]="_nrrdFormatUnknown_read", err[AIR_STRLEN_MED];

  /* insert code here, and remove error handling below */

  sprintf(err, "%s: ERROR!!! trying to read unknown format", me);
  biffAdd(NRRD, err);
  return 1;
}

int
_nrrdFormatUnknown_write(FILE *file, const Nrrd *nrrd, NrrdIO *nio) {
  char me[]="_nrrdFormatUnknown_write", err[AIR_STRLEN_MED];

  /* insert code here, and remove error handling below */

  sprintf(err, "%s: ERROR!!! trying to write unknown format", me);
  biffAdd(NRRD, err);
  return 1;
}

const NrrdFormat
_nrrdFormatUnknown = {
  "unknown",
  AIR_FALSE,  /* isImage */
  AIR_TRUE,   /* readable */
  AIR_FALSE,  /* usesDIO */
  _nrrdFormatUnknown_available,
  _nrrdFormatUnknown_nameLooksLike,
  _nrrdFormatUnknown_fitsInto,
  _nrrdFormatUnknown_contentStartsLike,
  _nrrdFormatUnknown_read,
  _nrrdFormatUnknown_write
};

const NrrdFormat *const
nrrdFormatUnknown = &_nrrdFormatUnknown;

const NrrdFormat *const
nrrdFormatArray[NRRD_FORMAT_TYPE_MAX+1] = {
  &_nrrdFormatUnknown,
  &_nrrdFormatNRRD,
  &_nrrdFormatPNM,
  &_nrrdFormatPNG,
  &_nrrdFormatVTK,
  &_nrrdFormatText,
  &_nrrdFormatEPS
};
