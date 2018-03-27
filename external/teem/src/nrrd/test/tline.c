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

extern int _nrrdOneLine(int *lenP, NrrdIO *io, FILE *file);

FILE *
myopen(char *name) {

  if (!strcmp(name, "-")) {
    return stdin;
  } else {
    return fopen(name, "r");
  }
}

void
myclose(FILE *file) {

  if (file != stdin) {
    fclose(file);
  }
  return;
}


int
main(int argc, char *argv[]) {
  char *me, *fileS;
  FILE *file;
  int len;
  NrrdIO *io;

  me = argv[0];
  if (2 != argc) {
    /*                       0   1   (2) */
    fprintf(stderr, "usage: %s <file>\n", me);
    exit(1);
  }
  fileS = argv[1];
  if (!( file = myopen(fileS) )) {
    fprintf(stderr, "%s: couldn't open \"%s\" for reading\n", me, fileS);
    exit(1);
  }
  io = nrrdIONew();
  do {
    if (_nrrdOneLine(&len, io, file)) {
      fprintf(stderr, "%s: trouble:\n%s", me, biffGet(NRRD));
      exit(1);
    }
    if (len) {
      printf("%2d   |%s|\n", len, io->line);
    }
  } while(len > 0);
  nrrdIONix(io);
  myclose(file);

  exit(0);
}
