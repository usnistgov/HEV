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

#include "ten.h"
#include "tenPrivate.h"

#define INFO "Generate postscript renderings of 2D glyphs"
char *_tend_ellipseInfoL =
  (INFO
   ".  Not much to look at here.");

int
tend_ellipseDoit(FILE *file, Nrrd *nten, float gscale, float cthresh) {
  int sx, sy, x, y;
  float aspect, scale, minX, minY, maxX, maxY, px, py, 
    conf, Dxx, Dxy, Dyy, (*lup)(const void*, size_t);

  lup = nrrdFLookup[nten->type];
  sx = nten->axis[1].size;
  sy = nten->axis[2].size;
  aspect = sx/sy;
  if (aspect > 7.5/10) {
    /* image has a wider aspect ratio than safely printable page area */
    minX = 0.5;
    maxX = 8.0;
    minY = 5.50 - 7.5*sy/sx/2;
    maxY = 5.50 + 7.5*sy/sx/2;
    scale = 7.5/sx;
  } else {
    /* image is taller ... */
    minX = 4.25 - 10.0*sx/sy/2;
    maxX = 4.25 + 10.0*sx/sy/2;
    minY = 0.5;
    maxY = 10.5;
    scale = 10.0/sy;
  }
  minX *= 72; minY *= 72;
  maxX *= 72; maxY *= 72;
  scale *= 72;

  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(file, "%%%%Creator: tend ellipse\n");
  fprintf(file, "%%%%Title: blah blah blah\n");
  fprintf(file, "%%%%Pages: 1\n");
  fprintf(file, "%%%%BoundingBox: %d %d %d %d\n",
	  (int)floor(minX), (int)floor(minY),
	  (int)ceil(maxX), (int)ceil(maxY));
  fprintf(file, "%%%%HiResBoundingBox: %g %g %g %g\n", 
	  minX, minY, maxX, maxY);
  fprintf(file, "%%%%EndComments\n");
  fprintf(file, "%%%%BeginProlog\n");
  fprintf(file, "%%%%EndProlog\n");
  fprintf(file, "%%%%Page: 1 1\n");
  fprintf(file, "gsave\n");
  for (y=0; y<sy; y++) {
    py = NRRD_CELL_POS(minY, maxY, sy, sy-1-y);
    for (x=0; x<sx; x++) {
      conf = lup(nten->data, 0 + 4*(x + sx*y));
      if (conf > cthresh) {
	Dxx = lup(nten->data, 1 + 4*(x + sx*y));
	Dxy = lup(nten->data, 2 + 4*(x + sx*y));
	Dyy = lup(nten->data, 3 + 4*(x + sx*y));
	px = NRRD_CELL_POS(minX, maxX, sx, x);
	fprintf(file, "gsave\n");
	fprintf(file, "matrix currentmatrix\n");
	fprintf(file, "[%g %g %g %g %g %g] concat\n",
		Dxx, -Dxy, -Dxy, Dyy, px, py);
	fprintf(file, "0 0 %g 0 360 arc closepath\n", gscale);
	fprintf(file, "setmatrix\n");
	fprintf(file, "0 setgray fill\n");
	fprintf(file, "grestore\n");
      }
    }
  }
  fprintf(file, "grestore\n");
  
  return 0;
}

int
tend_ellipseMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr;
  airArray *mop;

  Nrrd *nten;
  char *outS;
  float gscale, cthresh;
  FILE *fout;

  mop = airMopNew();

  hestOptAdd(&hopt, "ctr", "conf thresh", airTypeFloat, 1, 1, &cthresh, "0.5",
	     "Glyphs will be drawn only for tensors with confidence "
	     "values greater than this threshold");
  hestOptAdd(&hopt, "gsc", "scale", airTypeFloat, 1, 1, &gscale,
	     "1", "over-all glyph size");

  /* input/output */
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nten, "-",
	     "image of 2D tensors", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
	     "output PostScript file");

  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_ellipseInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!(3 == nten->dim && 4 == nten->axis[0].size)) {
    fprintf(stderr, "%s: didn't get a 3-D 4-by-X-by-Y 2D tensor array\n", me);
    airMopError(mop); return 1;
  }
  if (!(fout = airFopen(outS, stdout, "wb"))) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing\n", me, outS);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, fout, (airMopper)airFclose, airMopAlways);

  tend_ellipseDoit(fout, nten, gscale, cthresh);

  airMopOkay(mop);
  return 0;
}
/* TEND_CMD(glyph, INFO); */
unrrduCmd tend_ellipseCmd = { "ellipse", INFO, tend_ellipseMain };
