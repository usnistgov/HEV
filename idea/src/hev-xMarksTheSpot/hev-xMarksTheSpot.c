/*
 * XMarksTheSpot.c
 *
 * Steve Satterfield
 * 6/13/2016
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

void
usage() {


  fprintf(stderr, "\n\
    XmarksTheSpot inputTable.txt\n\
\n\
      Reads x,y,z location from stdin.\n\
      Compares location with spots defined in input.txt and match, the\n\
      corresponding string is passed to Bash for execution in background.\n\
\n\
      inputTable.txt:\n\
\n\
         xmin ymin zmin xmax ymax zmax  commandString\n\
         <repeat>\n\
\n\
       Typically, the head position is used. When the user navigates to a\n\
       spot the command is executed once. The user must then navigate away\n\
       and back again to run the command a second time.\n\
\n\
");

  exit(1);

}


#define SPOT_MAX 100

double xmin[SPOT_MAX],ymin[SPOT_MAX],zmin[SPOT_MAX];
double xmax[SPOT_MAX],ymax[SPOT_MAX],zmax[SPOT_MAX];

char *cmd[SPOT_MAX];
int execOnceFlag[SPOT_MAX];

char line[1000];

char *trimLeadingWhiteSpace(char *inString, int length)
{
  int l;
  l=length;
  while (l-- > 0) {
    if (isspace(*inString) == 0) break;
    inString++;
  }
  return(inString);
}

void trimTrailingWhiteSpace(char *inString, int length)
{
  char *p;
  int l;
  l=length;
  p=inString+length-1;
  while (l-- > 0) {
    if (isspace(*p) == 0) break;
    p--;
  }
  p++;
  *p=0;
}




int
main(int argc, char *argv[])
{
  FILE *fp;
  char *linePtr, *valueAsString;
  int i,n,l,inCount,msize;
  float x,y,z;

  if (argc != 2) {
    usage();
  }

  for (i=0; i<SPOT_MAX; i++) {
    execOnceFlag[i]=0;
  }
    

  /* Read command table */

  if ((fp=fopen(argv[1],"r")) == NULL) {
    fprintf(stderr,"Error reading '%s'\n",argv[1]);
    exit(1);
  }

  i=0;
  while (fgets(line, 1000, fp) != NULL) {
    linePtr = line;

    l=strnlen(linePtr, 1000);
    trimTrailingWhiteSpace(linePtr, l);

    l=strnlen(linePtr, 1000);
    linePtr=trimLeadingWhiteSpace(linePtr, l);

    if (l == 0) {continue;}
    fflush(stdout);
    if (*linePtr == '#') {continue;}


    valueAsString = strsep(&linePtr, " \t");
    xmin[i]=atof(valueAsString);


    l=strnlen(linePtr, 1000);
    linePtr=trimLeadingWhiteSpace(linePtr, l);
    valueAsString = strsep(&linePtr, " \t");
    ymin[i]=atof(valueAsString);

    l=strnlen(linePtr, 1000);
    linePtr=trimLeadingWhiteSpace(linePtr, l);
    valueAsString = strsep(&linePtr, " \t");
    zmin[i]=atof(valueAsString);


    l=strnlen(linePtr, 1000);
    linePtr=trimLeadingWhiteSpace(linePtr, l);
    valueAsString = strsep(&linePtr, " \t");
    xmax[i]=atof(valueAsString);

    l=strnlen(linePtr, 1000);
    linePtr=trimLeadingWhiteSpace(linePtr, l);
    valueAsString = strsep(&linePtr, " \t");
    ymax[i]=atof(valueAsString);

    linePtr=trimLeadingWhiteSpace(linePtr, l);
    valueAsString = strsep(&linePtr, " \t");
    zmax[i]=atof(valueAsString);

    l=strnlen(linePtr, 1000);
    linePtr=trimLeadingWhiteSpace(linePtr, l);
    //printf("vals %d: %g %g %g   %g %g %g '%s'\n", i,
    //	   xmin[i], ymin[i], zmin[i], xmax[i], ymax[i], zmax[i], linePtr);


    msize=l+7;
    cmd[i]=malloc(msize);
    //    strncpy(cmd[i], linePtr, l);
    snprintf(cmd[i], msize, "( %s ) &", linePtr);

    i++;
    fflush(stdout);

    

  }
  n=i;
  fclose(fp);
  
  /*
  for (i=0; i<n; i++) {
    printf("[%d] spot: %g %g %g   %g %g %g cmd: '%s'\n", i,
    	   xmin[i], ymin[i], zmin[i], xmax[i], ymax[i], zmax[i], cmd[i]);

  }
  */

  while (fgets(line, 1000, stdin) != NULL) {
    inCount=sscanf(line, "%g %g %g\n", &x,&y,&z);
    if (inCount==3) {
      //printf("input is %g %g %g\n",x,y,z);

      for (i=0; i<n; i++) {
	if ((x>xmin[i] && x<xmax[i]) &&
	    (y>ymin[i] && y<ymax[i]) &&
	    (z>zmin[i] && z<zmax[i])) {
	  break;
	}
	execOnceFlag[i]=0;
      }

      //printf("i=%d\n",i);
      if (i<n) {
	if (execOnceFlag[i]==0) {
	  //printf("Spot %d found\n",i);
	  execOnceFlag[i]=1;
	  system(cmd[i]);
	  sleep(5);
	} else {
	  //printf("still on spot %d\n",i);
	}
      } else {
	//printf("not on a spot\n");
      }

    } else {
      fprintf(stderr,"not enough input, n= %d\n",n);
    }
    fflush(stdout);
  }
      


}

