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

/*
******** nrrdHisto()
**
** makes a 1D histogram of a given size and type
**
** pre-NrrdRange policy:
** this looks at nin->min and nin->max to see if they are both non-NaN.
** If so, it uses these as the range of the histogram, otherwise it
** finds the min and max present in the volume.  If nin->min and nin->max
** are being used as the histogram range, then values which fall outside
** this are ignored (they don't contribute to the histogram).
**
** post-NrrdRange policy:
*/
int
nrrdHisto(Nrrd *nout, const Nrrd *nin, const NrrdRange *_range,
	  const Nrrd *nwght, int bins, int type) {
  char me[]="nrrdHisto", func[]="histo", err[AIR_STRLEN_MED];
  int idx;
  size_t I, num;
  airArray *mop;
  NrrdRange *range;
  double min, max, eps, val, count, incr, (*lup)(const void *v, size_t I);

  if (!(nin && nout)) {
    /* _range and nwght can be NULL */
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!(bins > 0)) {
    sprintf(err, "%s: bins value (%d) invalid", me, bins);
    biffAdd(NRRD, err); return 1;
  }
  if (airEnumValCheck(nrrdType, type) || nrrdTypeBlock == type) {
    sprintf(err, "%s: invalid nrrd type %d", me, type);
    biffAdd(NRRD, err); return 1;
  }
  if (nwght) {
    if (nout==nwght) {
      sprintf(err, "%s: nout==nwght disallowed", me);
      biffAdd(NRRD, err); return 1;
    }
    if (nrrdTypeBlock == nwght->type) {
      sprintf(err, "%s: nwght type %s invalid", me,
	      airEnumStr(nrrdType, nrrdTypeBlock));
      biffAdd(NRRD, err); return 1;
    }
    if (!nrrdSameSize(nin, nwght, AIR_TRUE)) {
      sprintf(err, "%s: nwght size mismatch with nin", me);
      biffAdd(NRRD, err); return 1;
    }
    lup = nrrdDLookup[nwght->type];
  } else {
    lup = NULL;
  }

  if (nrrdMaybeAlloc(nout, type, 1, bins)) {
    sprintf(err, "%s: failed to alloc histo array (len %d)", me, bins);
    biffAdd(NRRD, err); return 1;
  }
  mop = airMopNew();
  /* nout->axis[0].size set */
  nout->axis[0].spacing = AIR_NAN;
  if (nout && AIR_EXISTS(nout->axis[0].min) && AIR_EXISTS(nout->axis[0].max)) {
    /* HEY: total hack to externally nail down min and max of histogram:
       use the min and max already set on axis[0] */
    min = nout->axis[0].min;
    max = nout->axis[0].max;
  } else {
    if (_range) {
      range = nrrdRangeCopy(_range);
      nrrdRangeSafeSet(range, nin, nrrdBlind8BitRangeState);
    } else {
      range = nrrdRangeNewSet(nin, nrrdBlind8BitRangeState);
    }
    airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    min = range->min;
    max = range->max;
    nout->axis[0].min = min;
    nout->axis[0].max = max;
  }
  eps = (min == max ? 1.0 : 0.0);
  nout->axis[0].center = nrrdCenterCell;
  /* nout->axis[0].label set below */
  
  /* make histogram */
  num = nrrdElementNumber(nin);
  for (I=0; I<num; I++) {
    val = nrrdDLookup[nin->type](nin->data, I);
    if (AIR_EXISTS(val)) {
      if (val < min || val > max+eps) {
	/* value is outside range; ignore it */
	continue;
      }
      if (AIR_IN_CL(min, val, max)) {
	AIR_INDEX(min, val, max+eps, bins, idx);
	/*
	printf("!%s: %d: index(%g, %g, %g, %d) = %d\n", 
	       me, (int)I, min, val, max, bins, idx);
	*/
	/* count is a double in order to simplify clamping the
	   hit values to the representable range for nout->type */
	count = nrrdDLookup[nout->type](nout->data, idx);
	incr = nwght ? lup(nwght->data, I) : 1;
	count = nrrdDClamp[nout->type](count + incr);
	nrrdDInsert[nout->type](nout->data, idx, count);
      }
    }
  }

  if (nrrdContentSet(nout, func, nin, "%d", bins)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  AIR_FREE(nout->axis[0].label);
  nout->axis[0].label = airStrdup(nout->content);

  airMopOkay(mop);
  return 0;
}

int
nrrdHistoDraw(Nrrd *nout, const Nrrd *nin, int sy, int showLog, double max) {
  char me[]="nrrdHistoDraw", func[]="dhisto", err[AIR_STRLEN_MED],
    cmt[AIR_STRLEN_MED];
  int k, sx, x, y, maxhitidx, E,
    numticks, *Y, *logY, tick, *ticks;
  double hits, maxhits, usemaxhits;
  unsigned char *pgmData;
  airArray *mop;

  if (!(nin && nout && sy > 0)) {
    sprintf(err, "%s: invalid args", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!(1 == nin->dim && nrrdTypeBlock != nin->type)) {
    if (1 != nin->dim) {
      sprintf(err, "%s: nrrd can\'t be a histogram, dim=%d, not 1", 
	      me, nin->dim);
    } else {
      sprintf(err, "%s: nrrd can\'t be a histogram because it is %s type",
	      me, airEnumStr(nrrdType, nrrdTypeBlock));
    }
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdHasNonExist(nin)) {
    sprintf(err, "%s: given nrrd has non-existent values", me);
    biffAdd(NRRD, err); return 1;
  }
  sx = nin->axis[0].size;
  if (nrrdPGM(nout, sx, sy)) {
    sprintf(err, "%s: failed to allocate histogram image", me);
    biffAdd(NRRD, err); return 1;
  }
  /* perhaps I should be using nrrdAxisInfoCopy */
  nout->axis[0].spacing = nout->axis[1].spacing = AIR_NAN;
  nout->axis[0].min = nin->axis[0].min;
  nout->axis[0].max = nin->axis[0].max;
  nout->axis[0].center = nout->axis[1].center = nrrdCenterCell;
  nout->axis[0].label = airStrdup(nin->axis[0].label);
  AIR_FREE(nout->axis[1].label);
  nrrdPeripheralInit(nout);
  pgmData = nout->data;
  maxhits = maxhitidx = 0;
  for (x=0; x<sx; x++) {
    hits = nrrdDLookup[nin->type](nin->data, x);
    if (maxhits < hits) {
      maxhits = hits;
      maxhitidx = x;
    }
  }
  if (AIR_EXISTS(max) && max > 0) {
    usemaxhits = max;
  } else {
    usemaxhits = maxhits;
  }
  nout->axis[1].min = usemaxhits;
  nout->axis[1].max = 0;
  numticks = log10(usemaxhits + 1);
  mop = airMopNew();
  ticks = (int*)calloc(numticks, sizeof(int));
  airMopMem(mop, &ticks, airMopAlways);
  Y = (int*)calloc(sx, sizeof(int));
  airMopMem(mop, &Y, airMopAlways);
  logY = (int*)calloc(sx, sizeof(int));
  airMopMem(mop, &logY, airMopAlways);
  if (!(ticks && Y && logY)) {
    sprintf(err, "%s: failed to allocate temp arrays", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  for (k=0; k<numticks; k++) {
    AIR_INDEX(0, log10(pow(10,k+1) + 1), log10(usemaxhits+1), sy, ticks[k]);
  }
  for (x=0; x<sx; x++) {
    hits = nrrdDLookup[nin->type](nin->data, x);
    AIR_INDEX(0, hits, usemaxhits, sy, Y[x]);
    AIR_INDEX(0, log10(hits+1), log10(usemaxhits+1), sy, logY[x]);
    /* printf("%d -> %d,%d", x, Y[x], logY[x]); */
  }
  for (y=0; y<sy; y++) {
    tick = 0;
    for (k=0; k<numticks; k++)
      tick |= ticks[k] == y;
    for (x=0; x<sx; x++) {
      pgmData[x + sx*(sy-1-y)] = 
	(!showLog
	 ? (y >= Y[x] ? 0 : 255)
	 : (y >= logY[x]       /* above log curve                       */
	    ? (!tick ? 0       /*                    not on tick mark   */
	       : 255)          /*                    on tick mark       */
	    : (y >= Y[x]       /* below log curve, above normal curve   */
	       ? (!tick ? 128  /*                    not on tick mark   */
		  : 0)         /*                    on tick mark       */
	       :255            /* below log curve, below normal curve */
	       )
	    )
	 );
    }
  }
  E = AIR_FALSE;
  sprintf(cmt, "min value: %g\n", nout->axis[0].min);
  if (!E) E |= nrrdCommentAdd(nout, cmt);
  sprintf(cmt, "max value: %g\n", nout->axis[0].max);
  if (!E) E |= nrrdCommentAdd(nout, cmt);
  sprintf(cmt, "max hits: %g, in bin %d, around value %g\n",
	  maxhits, maxhitidx, nrrdAxisPos(nout, 0, maxhitidx));
  if (!E) E |= nrrdCommentAdd(nout, cmt);
  if (!E) E |= nrrdContentSet(nout, func, nin, "%d", sy);
  if (E) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }

  /* bye */
  airMopOkay(mop);
  return 0;
}

/*
******** nrrdHistoAxis
**
** replace scanlines along one scanline with a histogram of the scanline
**
** this looks at nin->min and nin->max to see if they are both non-NaN.
** If so, it uses these as the range of the histogram, otherwise it
** finds the min and max present in the volume
**
** By its very nature, and by the simplicity of this implemention,
** this can be a slow process due to terrible memory locality.  User
** may want to permute axes before and after this, but that can be
** slow too...  
*/
int
nrrdHistoAxis(Nrrd *nout, const Nrrd *nin, const NrrdRange *_range, 
	      int ax, int bins, int type) {
  char me[]="nrrdHistoAxis", func[]="histax", err[AIR_STRLEN_MED];
  int hidx, d, map[NRRD_DIM_MAX], size[NRRD_DIM_MAX];
  unsigned int szIn[NRRD_DIM_MAX], szOut[NRRD_DIM_MAX],
    coordIn[NRRD_DIM_MAX], coordOut[NRRD_DIM_MAX];
  size_t I, hI, num;
  double val, count;
  airArray *mop;
  NrrdRange *range;

  if (!(nin && nout)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!(bins > 0)) {
    sprintf(err, "%s: bins value (%d) invalid", me, bins);
    biffAdd(NRRD, err); return 1;
  }
  if (airEnumValCheck(nrrdType, type) || nrrdTypeBlock == type) {
    sprintf(err, "%s: invalid nrrd type %d", me, type);
    biffAdd(NRRD, err); return 1;
  }
  if (!AIR_IN_CL(0, ax, nin->dim-1)) {
    sprintf(err, "%s: axis %d is not in range [0,%d]", me, ax, nin->dim-1);
    biffAdd(NRRD, err); return 1;
  }
  mop = airMopNew();
  if (_range) {
    range = nrrdRangeCopy(_range);
    nrrdRangeSafeSet(range, nin, nrrdBlind8BitRangeState);
  } else {
    range = nrrdRangeNewSet(nin, nrrdBlind8BitRangeState);
  }
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  size[ax] = bins;
  if (nrrdMaybeAlloc_nva(nout, type, nin->dim, size)) {
    sprintf(err, "%s: failed to alloc output nrrd", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  
  /* copy axis information */
  for (d=0; d<nin->dim; d++) {
    map[d] = d != ax ? d : -1;
  }
  nrrdAxisInfoCopy(nout, nin, map, NRRD_AXIS_INFO_NONE);
  /* axis ax now has to be set manually */
  nout->axis[ax].size = bins;
  nout->axis[ax].spacing = AIR_NAN; /* min and max convey the information */
  nout->axis[ax].min = range->min;
  nout->axis[ax].max = range->max;
  nout->axis[ax].center = nrrdCenterCell;
  if (nin->axis[ax].label) {
    nout->axis[ax].label = calloc(strlen("histax()")
				  + strlen(nin->axis[ax].label)
				  + 1, sizeof(char));
    if (nout->axis[ax].label) {
      sprintf(nout->axis[ax].label, "histax(%s)", nin->axis[ax].label);
    } else {
      sprintf(err, "%s: couldn't allocate output label", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
  } else {
    nout->axis[ax].label = NULL;
  }

  /* the skinny: we traverse the input samples in linear order, and
     increment the bin in the histogram for the scanline we're in.
     This is not terribly clever, and the memory locality is a
     disaster */
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  nrrdAxisInfoGet_nva(nout, nrrdAxisInfoSize, szOut);
  memset(coordIn, 0, NRRD_DIM_MAX*sizeof(unsigned int));
  num = nrrdElementNumber(nin);
  for (I=0; I<num; I++) {
    /* get input nrrd value and compute its histogram index */
    val = nrrdDLookup[nin->type](nin->data, I);
    if (AIR_EXISTS(val)) {
      AIR_INDEX(range->min, val, range->max, bins, hidx);
      if (AIR_IN_CL(0, hidx, bins-1)) {
	memcpy(coordOut, coordIn, nin->dim*sizeof(int));
	coordOut[ax] = hidx;
	NRRD_INDEX_GEN(hI, coordOut, szOut, nout->dim);
	count = nrrdDLookup[nout->type](nout->data, hI);
	count = nrrdDClamp[nout->type](count + 1);
	nrrdDInsert[nout->type](nout->data, hI, count);
      }
    }
    NRRD_COORD_INCR(coordIn, szIn, nin->dim, 0);
  }

  if (nrrdContentSet(nout, func, nin, "%d,%d", ax, bins)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  nrrdPeripheralInit(nout);
  airMopOkay(mop); 
  return 0;
}

int 
nrrdHistoJoint(Nrrd *nout, const Nrrd *const *nin,
	       const NrrdRange *const *_range, int numNin,
	       const Nrrd *nwght, const int *bins,
	       int type, const int *clamp) {
  char me[]="nrrdHistoJoint", func[]="jhisto", err[AIR_STRLEN_MED];
  int i, d, coord[NRRD_DIM_MAX], skip, hadContent, totalContentStrlen, len=0;
  double val, count, incr, (*lup)(const void *v, size_t I);
  size_t Iin, Iout, numEl;
  airArray *mop;
  NrrdRange **range;

  /* error checking */
  /* nwght can be NULL -> weighting is constant 1.0 */
  if (!(nout && nin && bins && clamp)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!(numNin >= 1)) {
    sprintf(err, "%s: need numNin >= 1 (not %d)", me, numNin);
    biffAdd(NRRD, err); return 1;
  }
  if (numNin > NRRD_DIM_MAX) {
    sprintf(err, "%s: can only deal with up to %d nrrds (not %d)", me,
	    NRRD_DIM_MAX, numNin);
    biffAdd(NRRD, err); return 1;
  }
  for (i=0; i<numNin; i++) {
    if (!(nin[i])) {
      sprintf(err, "%s: input nrrd #%d NULL", me, i);
      biffAdd(NRRD, err); return 1;
    }
    if (nout==nin[i]) {
      sprintf(err, "%s: nout==nin[%d] disallowed", me, i);
      biffAdd(NRRD, err); return 1;
    }
    if (nrrdTypeBlock == nin[i]->type) {
      sprintf(err, "%s: nin[%d] type %s invalid", me, i,
	      airEnumStr(nrrdType, nrrdTypeBlock));
      biffAdd(NRRD, err); return 1;
    }
  }
  if (airEnumValCheck(nrrdType, type) || nrrdTypeBlock == type) {
    sprintf(err, "%s: invalid nrrd type %d", me, type);
    biffAdd(NRRD, err); return 1;
  }
  mop = airMopNew();
  range = (NrrdRange**)calloc(numNin, sizeof(NrrdRange*));
  airMopAdd(mop, range, airFree, airMopAlways);
  for (d=0; d<numNin; d++) {
    if (_range && _range[d]) {
      range[d] = nrrdRangeCopy(_range[d]);
      nrrdRangeSafeSet(range[d], nin[d], nrrdBlind8BitRangeState);
    } else {
      range[d] = nrrdRangeNewSet(nin[d], nrrdBlind8BitRangeState);
    }
    airMopAdd(mop, range[d], (airMopper)nrrdRangeNix, airMopAlways);
  }
  for (d=0; d<numNin; d++) {
    if (!nin[d]) {
      sprintf(err, "%s: input nrrd[%d] NULL", me, d);
      biffAdd(NRRD, err); return 1;
    }
    if (!(bins[d] >= 1)) {
      sprintf(err, "%s: need bins[%d] >= 1 (not %d)", me, d, bins[d]);
      biffAdd(NRRD, err); return 1;
    }
    if (d && !nrrdSameSize(nin[0], nin[d], AIR_TRUE)) {
      sprintf(err, "%s: nin[%d] size mismatch with nin[0]", me, d);
      biffAdd(NRRD, err); return 1;
    }
  }
  
  /* check nwght */
  if (nwght) {
    if (nout==nwght) {
      sprintf(err, "%s: nout==nwght disallowed", me);
      biffAdd(NRRD, err); return 1;
    }
    if (nrrdTypeBlock == nwght->type) {
      sprintf(err, "%s: nwght type %s invalid", me,
	      airEnumStr(nrrdType, nrrdTypeBlock));
      biffAdd(NRRD, err); return 1;
    }
    if (!nrrdSameSize(nin[0], nwght, AIR_TRUE)) {
      sprintf(err, "%s: nwght size mismatch with nin[0]", me);
      biffAdd(NRRD, err); return 1;
    }
    lup = nrrdDLookup[nwght->type];
  } else {
    lup = NULL;
  }

  /* allocate output nrrd */
  if (nrrdMaybeAlloc_nva(nout, type, numNin, bins)) {
    sprintf(err, "%s: couldn't allocate output histogram", me);
    biffAdd(NRRD, err); return 1;
  }
  hadContent = 0;
  totalContentStrlen = 0;
  for (d=0; d<numNin; d++) {
    nout->axis[d].size = bins[d];
    nout->axis[d].spacing = AIR_NAN;
    nout->axis[d].min = range[d]->min;
    nout->axis[d].max = range[d]->max;
    nout->axis[d].center = nrrdCenterCell;
    if (nin[d]->content) {
      hadContent = 1;
      totalContentStrlen += strlen(nin[d]->content);
      nout->axis[d].label = calloc(strlen("histo(,)")
				   + strlen(nin[d]->content)
				   + 11
				   + 1, sizeof(char));
      if (nout->axis[d].label) {
	sprintf(nout->axis[d].label, "histo(%s,%d)", nin[d]->content, bins[d]);
      } else {
	sprintf(err, "%s: couldn't allocate output label #%d", me, d);
	biffAdd(NRRD, err); return 1;
      }
    } else {
      AIR_FREE(nout->axis[d].label);
      totalContentStrlen += 2;
    }
  }

  /* the skinny */
  numEl = nrrdElementNumber(nin[0]);
  for (Iin=0; Iin<numEl; Iin++) {
    skip = 0;
    /*
    printf("%s: Iin = " NRRD_BIG_INT_PRINTF "; ", me, Iin); fflush(stdout);
    */
    for (d=0; d<numNin; d++) {
      val = nrrdDLookup[nin[d]->type](nin[d]->data, Iin);
      /* printf("val[%d] = %g", d, val); fflush(stdout); */
      if (!AIR_EXISTS(val)) {
	/* coordinate d in the joint histo can't be determined
	   if nin[d] has a non-existent value here */
	skip = 1;
	break;
      }
      if (!AIR_IN_CL(range[d]->min, val, range[d]->max)) {
	if (clamp[d]) {
	  val = AIR_CLAMP(range[d]->min, val, range[d]->max);
	} else {
	  skip = 1;
	  break;
	}
      }
      AIR_INDEX(range[d]->min, val, range[d]->max, bins[d], coord[d]);
      /* printf(" -> coord = %d; ", coord[d]); fflush(stdout); */
    }
    if (skip)
      continue;
    /* printf("\n"); */
    NRRD_INDEX_GEN(Iout, coord, bins, numNin);
    count = nrrdDLookup[nout->type](nout->data, Iout);
    incr = nwght ? lup(nwght->data, Iin) : 1.0;
    count = nrrdDClamp[nout->type](count + incr);
    nrrdDInsert[nout->type](nout->data, Iout, count);
  }

  /* HEY: switch to nrrdContentSet? */
  if (hadContent) {
    nout->content = calloc(strlen(func) + strlen("()")
			   + numNin*strlen(",")
			   + totalContentStrlen
			   + 1, sizeof(char));
    if (nout->content) {
      sprintf(nout->content, "%s(", func);
      for (d=0; d<numNin; d++) {
	len = strlen(nout->content);
	strcpy(nout->content + len,
	       nin[d]->content ? nin[d]->content : "?");
	len = strlen(nout->content);
	nout->content[len] = d < numNin-1 ? ',' : ')';
      }
      nout->content[len+1] = '\0';
    } else {
      sprintf(err, "%s: couldn't allocate output content", me);
      biffAdd(NRRD, err); return 1;
    }
  }

  return 0;
}

