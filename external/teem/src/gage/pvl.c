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

#include "gage.h"
#include "privateGage.h"

/*
******** gageVolumeCheck()
**
** checks whether a given volume is valid for the given kind
** and the given parameter settings in the context
*/
int
gageVolumeCheck (gageContext *ctx, Nrrd *nin, gageKind *kind) {
  char me[]="gageVolumeCheck", err[AIR_STRLEN_MED];
  gageShape shape;

  gageShapeReset(&shape);
  if (_gageShapeSet(ctx, &shape, nin, kind->baseDim)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(GAGE, err); return 1;;
  }
  return 0;
}

/*
******** gagePerVolumeNew()
**
** creates a new pervolume of a known kind, but nothing besides the
** answer array is allocated
**
** uses biff primarily because of the error checking in gageVolumeCheck()
*/
gagePerVolume *
gagePerVolumeNew (gageContext *ctx, Nrrd *nin, gageKind *kind) {
  char me[]="gagePerVolumeNew", err[AIR_STRLEN_MED];
  gagePerVolume *pvl;
  int i;

  if (!( nin && kind )) {
    sprintf(err, "%s: got NULL pointer", me);
    return NULL;
  }
  if (gageVolumeCheck(ctx, nin, kind)) {
    sprintf(err, "%s: problem with given volume", me);
    biffAdd(GAGE, err); return NULL;
  }
  pvl = (gagePerVolume *)calloc(1, sizeof(gagePerVolume));
  if (!pvl) {
    sprintf(err, "%s: couldn't alloc gagePerVolume", me);
    biffAdd(GAGE, err); return NULL;
  }
  pvl->thisIsACopy = AIR_FALSE;
  pvl->verbose = gageDefVerbose;
  pvl->kind = kind;
  pvl->query = 0;
  pvl->needD[0] = pvl->needD[1] = pvl->needD[2] = AIR_FALSE;
  pvl->nin = nin;
  pvl->padder = _gageStandardPadder;
  pvl->nixer = _gageStandardNixer;
  pvl->padInfo = NULL;
  pvl->npad = NULL;
  for (i=0; i<GAGE_PVL_FLAG_NUM; i++) {
    pvl->flag[i] = AIR_FALSE;
  }
  pvl->iv3 = pvl->iv2 = pvl->iv1 = NULL;
  pvl->lup = nrrdLOOKUP[nin->type];
  pvl->ans = (gage_t *)calloc(kind->totalAnsLen, sizeof(gage_t));
  if (!pvl->ans) {
    sprintf(err, "%s: couldn't alloc answer array", me);
    biffAdd(GAGE, err); return NULL;
  }
  pvl->flag[gagePvlFlagVolume] = AIR_TRUE;

  return pvl;
}

/*
** _gagePerVolumeCopy()
**
** copies a pervolume for use in a copied context, and probably
** should only be called by gageContextCopy()
*/
gagePerVolume *
_gagePerVolumeCopy (gagePerVolume *pvl, int fd) {
  char me[]="gagePerVolumeCopy", err[AIR_STRLEN_MED];
  gagePerVolume *nvl;
  
  nvl = (gagePerVolume *)calloc(1, sizeof(gagePerVolume));
  if (!nvl) {
    sprintf(err, "%s: couldn't create new pervolume", me);
    biffAdd(GAGE, err); return NULL;
  }
  /* we should probably restrict ourselves to gage API calls, but given the
     constant state of gage construction, this seems much simpler.
     Pointers are fixed below */
  memcpy(nvl, pvl, sizeof(gagePerVolume));
  nvl->thisIsACopy = AIR_TRUE;
  nvl->nin = NULL;             /* foil any repadding attempts */
  nvl->padder = NULL;          /* foil any repadding attempts */
  nvl->nixer = NULL;           /* foil any repadding attempts */
  nvl->padInfo = NULL;         /* foil any repadding attempts */
  /* the padded volume (nvl->npad) is the one that is shared between the 
     original and copied pervolumes; that pointer has already been copied */
  nvl->iv3 = (gage_t *)calloc(fd*fd*fd*nvl->kind->valLen, sizeof(gage_t));
  nvl->iv2 = (gage_t *)calloc(fd*fd*nvl->kind->valLen, sizeof(gage_t));
  nvl->iv1 = (gage_t *)calloc(fd*nvl->kind->valLen, sizeof(gage_t));
  nvl->ans = (gage_t *)calloc(nvl->kind->totalAnsLen, sizeof(gage_t));
  if (!( nvl->iv3 && nvl->iv2 && nvl->iv1 && nvl->ans )) {
    sprintf(err, "%s: couldn't allocate all caches", me);
    biffAdd(GAGE, err); return NULL;
  }
  
  return nvl;
}

/*
******** gagePerVolumeNix()
**
** uses the nixer to remove the padded volume, and frees all other
** dynamically allocated memory assocated with a pervolume
**
** does not use biff
*/
gagePerVolume *
gagePerVolumeNix (gagePerVolume *pvl) {

  AIR_FREE(pvl->iv3);
  AIR_FREE(pvl->iv2);
  AIR_FREE(pvl->iv1);
  if (!pvl->thisIsACopy && pvl->nixer) {
    pvl->nixer(pvl->npad, pvl->kind, pvl);
  }
  AIR_FREE(pvl->ans);
  AIR_FREE(pvl);
  return NULL;
}

/*
******** gagePadderSet()
**
*/
void
gagePadderSet (gageContext *ctx, gagePerVolume *pvl, gagePadder_t *padder) {

  if (pvl) {
    pvl->padder = padder;
    pvl->flag[gagePvlFlagPadder] = AIR_TRUE;
  }
}

/*
******** gageNixerSet()
**
*/
void
gageNixerSet (gageContext *ctx, gagePerVolume *pvl, gageNixer_t *nixer) {
  
  if (pvl) {
    pvl->nixer = nixer;
  }
}

/*
******** gageAnswerPointer()
**
** way of getting a pointer to a specific answer in a pervolume's ans array
**
** Basically just a wrapper around GAGE_ANSWER_POINTER with error checking
*/
gage_t *
gageAnswerPointer (gageContext *ctx, gagePerVolume *pvl, int measure) {
  gage_t *ret;

  if (pvl && !airEnumValCheck(pvl->kind->enm, measure)) {
    ret = GAGE_ANSWER_POINTER(pvl, measure);
  } else {
    ret = NULL;
  }
  return ret;
}

/*
******** gageQuerySet()
**
** sets a query in a pervolume.  Does recursive expansion of query
** to cover all prerequisite measures.  
**
** Sets: pvl->query
*/
int
gageQuerySet (gageContext *ctx, gagePerVolume *pvl, unsigned int query) {
  char me[]="gageQuerySet", err[AIR_STRLEN_MED];
  unsigned int mask, lastq, q;
  
  if (!( pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (pvl->thisIsACopy) {
    sprintf(err, "%s: can't operate on a pervolume copy", me);
    biffAdd(GAGE, err); return 1;
  }
  mask = (1U << (pvl->kind->queryMax+1)) - 1;
  if (query != (query & mask)) {
    sprintf(err, "%s: invalid bits set in query", me);
    biffAdd(GAGE, err); return 1;
  }
  pvl->query = query;
  if (pvl->verbose) {
    fprintf(stderr, "%s: original ", me);
    gageQueryPrint(stderr, pvl->kind, pvl->query);
  }
  /* recursive expansion of prerequisites */
  do {
    lastq = pvl->query;
    q = pvl->kind->queryMax+1;
    do {
      q--;
      if ((1<<q) & pvl->query)
	pvl->query |= pvl->kind->queryPrereq[q];
    } while (q);
  } while (pvl->query != lastq);
  if (pvl->verbose) {
    fprintf(stderr, "%s: expanded ", me);
    gageQueryPrint(stderr, pvl->kind, pvl->query);
  }
  pvl->flag[gagePvlFlagQuery] = AIR_TRUE;

  return 0;
}
