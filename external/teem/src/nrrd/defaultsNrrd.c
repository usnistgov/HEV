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
** these aren't "const"s because the user should be able to change
** default behavior- until a more sophisticated mechanism for this
** kind of control is developed, it seems simple and usable enough to
** have this be global state which we agree to treat nicely, as in,
** threads shouldn't be changing these willy-nilly.
**
** What IS a "default"?  A default is the assertion of a certain
** choice in situations where the user hasn't set it explicitly, but
** COULD.  The pad value in resampling is a good example: it is set by
** a constructor to nrrdDefRsmpPadValue, but the user can also set it
** explicitly.
*/

const NrrdEncoding *nrrdDefWriteEncoding = &_nrrdEncodingRaw;
int nrrdDefWriteBareText = AIR_TRUE;
int nrrdDefWriteCharsPerLine = 75;
int nrrdDefWriteValsPerLine = 8;
int nrrdDefRsmpBoundary = nrrdBoundaryBleed;
int nrrdDefRsmpType = nrrdTypeDefault;
double nrrdDefRsmpScale = 1.0;    /* these two should probably be the same */
double nrrdDefKernelParm0 = 1.0; 
int nrrdDefRsmpRenormalize = AIR_TRUE;
int nrrdDefRsmpRound = AIR_TRUE;
int nrrdDefRsmpClamp = AIR_TRUE;
double nrrdDefRsmpPadValue = 0.0;
int nrrdDefCenter = nrrdCenterNode;
double nrrdDefSpacing = 1.0;

/* these aren't really "defaults" because there's no other channel for
   specifying this information.  It is just global state.  Obviously,
   like defaults, they are not thread-safe if different threads ever
   set them differently. */
int nrrdStateVerboseIO = 1;
int nrrdStateBlind8BitRange = AIR_TRUE;
int nrrdStateMeasureType = nrrdTypeFloat;
int nrrdStateMeasureModeBins = 1024;
int nrrdStateMeasureHistoType = nrrdTypeFloat;
int nrrdStateAlwaysSetContent = AIR_TRUE;
int nrrdStateDisableContent = AIR_FALSE;
char *nrrdStateUnknownContent = NRRD_UNKNOWN;
int nrrdStateDisallowIntegerNonExist = AIR_TRUE;
int nrrdStateGrayscaleImage3D = AIR_FALSE;

/* should the acceptance (or not) of malformed NRRD header fields 
   embedded in PNM or text comments be controlled here? */

/* Are there other assumptions currently built into nrrd which could
   stand to be user-controllable? */

void
nrrdDefGetenv(void) {
  char *envS;
  int valI;
  
  if ((envS = getenv("NRRD_DEF_CENTER"))
      && (valI = airEnumVal(nrrdCenter, envS))) {
    nrrdDefCenter = valI;
  }
  if ((envS = getenv("NRRD_DEF_WRITE_BARE_TEXT"))
      && (valI = airEnumVal(airBool, envS))) {
    nrrdDefWriteBareText = valI;
  }
  return;
}

void
nrrdStateGetenv(void) {
  char *envS;
  int valI;
  
  if ((envS = getenv("NRRD_STATE_VERBOSE_IO"))
      && (-1 != (valI = airEnumVal(airBool, envS)))) {
    nrrdStateVerboseIO = valI;
  }
  if ((envS = getenv("NRRD_STATE_BLIND_8_BIT_RANGE"))
      && (-1 != (valI = airEnumVal(airBool, envS)))) {
    nrrdStateBlind8BitRange = valI;
  }
  if ((envS = getenv("NRRD_STATE_ALWAYS_SET_CONTENT"))
      && (-1 != (valI = airEnumVal(airBool, envS)))) {
    nrrdStateAlwaysSetContent = valI;
  }
  if ((envS = getenv("NRRD_STATE_DISABLE_CONTENT"))
      && (-1 != (valI = airEnumVal(airBool, envS)))) {
    nrrdStateDisableContent = valI;
  }
  if ((envS = getenv("NRRD_STATE_MEASURE_TYPE"))
      && (nrrdTypeUnknown != (valI = airEnumVal(nrrdType, envS)))) {
    nrrdStateMeasureType = valI;
  }
  if ((envS = getenv("NRRD_STATE_MEASURE_MODE_BINS"))
      && (1 == sscanf(envS, "%d", &valI))) {
    nrrdStateMeasureModeBins = valI;
  }
  if ((envS = getenv("NRRD_STATE_MEASURE_HISTO_TYPE"))
      && (nrrdTypeUnknown != (valI = airEnumVal(nrrdType, envS)))) {
    nrrdStateMeasureHistoType = valI;
  }
  return;
}

