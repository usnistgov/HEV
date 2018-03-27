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

#include "../nrrd.h"

#define BINS 1024
#define HGHT 800

int
main(int argc, char **argv) {
  Nrrd *nval, *nhist, *npgm;
  double *val;
  int i;
  
  nrrdAlloc(nval=nrrdNew(), nrrdTypeDouble, 1, BINS*BINS);
  val = nval->data;

  airSrand();
  for (i=0; i<BINS*BINS; i++) {
    val[i] = airRand();
  }
  
  nrrdHisto(nhist=nrrdNew(), nval, NULL, NULL, BINS, nrrdTypeInt);
  nrrdHistoDraw(npgm=nrrdNew(), nhist, HGHT, AIR_FALSE, 0.0);
  nrrdSave("hist.pgm", npgm, NULL);
  
  nrrdNuke(nval);
  nrrdNuke(nhist);
  nrrdNuke(npgm);

  return 0;
}
