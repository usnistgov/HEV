/*
 *       Scientific Applications & Visualization Group
 *            SAVG web page: http://math.nist.gov/mcsd/savg/
 *       Mathematical & Computational Science Division
 *            MCSD web page: http://math.nist.gov/mcsd/
 *       National Institute of Standards and Technology
 *            NIST web page: http://www.nist.gov/
 * 
 *  
 *  This software was developed at the National Institute of Standards
 *  and Technology by employees of the Federal Government in the course
 *  of their official duties. Pursuant to title 17 Section 105 of the
 *  United States Code this software is not subject to copyright
 *  protection and is in the public domain.  isolate is an experimental 
 *  system.  NIST assumes no responsibility whatsoever for its use by 
 *  other parties, and makes no guarantees, expressed or implied, about 
 *  its quality, reliability, or any other characteristic.
 *  
 *  We would appreciate acknowledgement if the software is used.
 * 
 */


/************************************************
 * savg-translate				*
 * Created by: Brandon M. Smith 
 * Original "gtb" file by: Harry Bullen		*
 * Created on: 6-17-04				*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-translate translates savg files 	*
 * according to the command line arguments, -x,	*
 * -y, and -z.  If no arguments are given, a    *
 * usage message if printed, as without and 	*
 * command line arguments gtb-translate cannot  *
 * do any thing.  Also all comments in the files*
 * are left intact.				*
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "feed.h"
#include "savgLineEvaluator.c"

#define ARRAY_CAPACITY (10000)
double storePoint[10000][3];
int connectivity[10000][50];
int trilist[50];
FILE* fp;
FILE* fp2;

/* Prints information on usage*/
void usage();
int newPoint(double tmpx, double tmpy, double tmpz, 
             int ntot);
void printInfo(int ntot, int nel);

int main(argc, argv)
int argc;
char** argv;
{
/*
 * INITIALIZATION
 */
	double x = 0, y = 0, z = 0; /* translation values */
	double tmpx = 0, tmpy = 0, tmpz = 0; /* storage for each point */
	char line[STRING_LENGTH];
	char templine[STRING_LENGTH];
	char assembleNewLine[STRING_LENGTH] = "";
	char tempString[20] = ""; 
	int pargc = 0;
	char **pargv  = NULL;
	int i,j,jj,jjj;
	char tempval[10];
	int ntot = 0;
	int conntot = 0;
	int nel = 0;
	int nnext = 0;
	int isit = 0;
/*
 * COMMAND LINE PROCESSING
  if(argc != 3) { 
    usage();
    exit(EXIT_FAILURE);
  }
 */

/*
 * BODY
 */
	if(!getLine(line))
	  return 0;

	fp = fopen(argv[1], "w");
	fp2 = fopen(argv[2], "w");
	
        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line,pargv,pargc))
	{
	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  //printf("keyword line: %s pargc %d\n",line,pargc);
	  nnext = 0;
	  connectivity[nel][0] = 0;
	  if( !getLine(line) )
	    return 0;

	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      dpf_charsToArgcArgv(line, &pargc, &pargv);

	      if (strcasecmp(pargv[0],"text")==0) {
	      }

	      else {
		tmpx = x + atof(pargv[0]); 
		tmpy = y + atof(pargv[1]);
		tmpz = z + atof(pargv[2]);
		//printf("%f %f %f\n",tmpx,tmpy,tmpz);
		// see if this is a new point
		isit = newPoint(tmpx,tmpy,tmpz,ntot);

		if (isit < 0) {
		  storePoint[ntot][0] = tmpx;
		  storePoint[ntot][1] = tmpy;
		  storePoint[ntot][2] = tmpz;
		  // add point number to element list
		  connectivity[nel][nnext+1] = ntot;
		  // keep track of nodes per element in first entry
		  connectivity[nel][0] = connectivity[nel][0] + 1;
		  ntot = ntot + 1;
		  nnext = nnext + 1;
		  //printf("new point: %f %f %f\n",tmpx,tmpy,tmpz);
		  //printf("connectivity: %d %d\n",connectivity[nel][0],connectivity[nel][nnext]);
		}
		else {
		  connectivity[nel][nnext+1] = isit;
		  // keep track of nodes per element in first entry
		  connectivity[nel][0] = connectivity[nel][0] + 1;
		  nnext = nnext + 1;
		}
	      }
	    }
	    if(!getLine(line)) {
	      // process the last element
	      nel = nel + 1;
	      // print information
	      
	      // print out file with node numbers and coordinates
	      printInfo(ntot, nel);
	      return 0;
	    }
	  }
	  // process next polygon
	  nel = nel + 1;
	  //printf("print connectivity: ");
	  //for (j = 0; j < connectivity[nel-1][0]+1; ++j) {
	  //  printf(" %d ",connectivity[nel-1][j]);
	  //}
	  //printf("\n");
	  //printf("done with this polygon\n");
	  //printf("next tri: %d %d %d\n",trilist[0],trilist[1],trilist[2]);
	}

	// print out file with node numbers and coordinates
	printInfo(ntot, nel);

        return 0;
}

/* Prints information on usage*/
void usage()
{
	fprintf(stderr,
	"ERROR: Incorrect command line arguments\n"
	"\n"
	"USAGE:\n"
	"\tsavg-graphFilter nodesfilename.savg connectivityname.savg\n"
	"\tfilenames are required.\n");
}

int newPoint(double tmpx, double tmpy, double tmpz, 
	     int ntot) {
  int isit = -1;
  int i;
  if (ntot > 0) {
    for (i = 0; i < ntot; ++i ) {
      if (fabs(tmpx - storePoint[i][0]) < 0.00001 &
	  fabs(tmpy - storePoint[i][1]) < 0.00001 &
	  fabs(tmpz - storePoint[i][2]) < 0.00001) {
	isit = i;
	//printf("found: %d\n",i);
	break;
      }
    }
  }
  return isit;
}

void printInfo(int ntot, int nel) {
  int i,j,jj,jjj,k;
  int nn;
  int onlist;
  int prev,nxt;
  int pnum;

  for (i = 0; i < ntot; ++i) {
    fprintf(fp, "%d   %f %f %f\n",i,storePoint[i][0],storePoint[i][1],storePoint[i][2]);
  }
  fclose(fp);

  // print graph
  onlist = 0;
  for (i = 0; i < ntot; ++i) {
    fprintf(fp2, "%d ",i);
    // loop through all element connectivities
    nn = 0;
    for (j = 0; j < nel; ++j) {
      pnum = connectivity[j][0];
      // does this polygon have node i in it
      onlist = -1;
      for (jj = 0; jj < connectivity[j][0]; ++jj) {
	if (connectivity[j][jj+1] == i) onlist = jj;
      }

      if (onlist > -1) {
	// find next and previous nodes
	if (onlist > 0) 
	  prev = connectivity[j][onlist];
	else
	  prev = connectivity[j][pnum];
	if (onlist < pnum-1)
	  nxt = connectivity[j][onlist+2];
	else
	  nxt = connectivity[j][1];

	// add prev and nxt to trilist
	onlist = -1;
	for (jj = 0; jj < nn; ++jj ) {
	  if (trilist[jj] == prev) onlist = 1;
	}
	if (onlist == -1) {
	  trilist[nn] = prev;
	  nn = nn + 1;
	}
	onlist = -1;
	for (jj = 0; jj < nn; ++jj ) {
	  if (trilist[jj] == nxt) onlist = 1;
	}
	if (onlist == -1) {
	  trilist[nn] = nxt;
	  nn = nn + 1;
	}
      }
    }
    
    for (k = 0; k < nn; ++k ) {
      fprintf(fp2, " %d ",trilist[k]);
    }
    fprintf(fp2, "\n");
  }

  fclose(fp2);
}
