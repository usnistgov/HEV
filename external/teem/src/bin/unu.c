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

#include <teem/unrrdu.h>

#define UNU "unu"

int
main(int argc, char **argv) {
  int i, ret;
  char *me, *argv0 = NULL, *err;
  hestParm *hparm;
  airArray *mop;

  me = argv[0];

  /* parse environmane variables first, in case they break nrrdDef*
     or nrrdState* variables in a way that nrrdSanity() should see */
  nrrdDefGetenv();
  nrrdStateGetenv();

  /* no harm done in making sure we're sane */
  if (!nrrdSanity()) {
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s: nrrd sanity check FAILED.\n", me);
    fprintf(stderr, "\n");
    fprintf(stderr, "  This means that either nrrd can't work on this "
	    "platform, or (more likely)\n");
    fprintf(stderr, "  there was an error in the compilation options "
	    "and variable definitions\n");
    fprintf(stderr, "  for how teem was built here.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  %s\n", err = biffGetDone(NRRD));
    fprintf(stderr, "\n");
    fprintf(stderr, "******************************************\n");
    fprintf(stderr, "******************************************\n");
    free(err);
    return 1;
  }

  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hparm->elideSingleEnumType = AIR_TRUE;
  hparm->elideSingleOtherType = AIR_TRUE;
  hparm->elideSingleOtherDefault = AIR_TRUE;
  hparm->elideSingleNonExistFloatDefault = AIR_TRUE;
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
  hparm->elideSingleEmptyStringDefault = AIR_TRUE;
  hparm->elideMultipleEmptyStringDefault = AIR_TRUE;
  hparm->columns = unrrduDefNumColumns;

  /* if there are no arguments, then we give general usage information */
  if (1 >= argc) {
    unrrduUsage("unu", hparm);
    airMopError(mop);
    exit(1);
  }
  /* else, we should see if they're asking for a command we know about */  
  for (i=0; unrrduCmdList[i]; i++) {
    if (!strcmp(argv[1], unrrduCmdList[i]->name))
      break;
  }
  /* unrrduCmdList[] is NULL-terminated */
  if (unrrduCmdList[i]) {
    /* yes, we have that command */
    /* initialize variables used by the various commands */
    argv0 = malloc(strlen(UNU) + strlen(argv[1]) + 2);
    airMopMem(mop, &argv0, airMopAlways);
    sprintf(argv0, "%s %s", UNU, argv[1]);

    /* run the individual unu program, saving its exit status */
    ret = unrrduCmdList[i]->main(argc-2, argv+2, argv0, hparm);
  } else {
    fprintf(stderr, "%s: unrecognized command: \"%s\"; type \"%s\" for "
	    "complete list\n", me, argv[1], me);
    ret = 1;
  }

  airMopDone(mop, ret);
  return ret;
}
