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
 * savg-reverse				        *
 * Created by: Adele Peskin                     *
 * Created on: 4-27-06				*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-reverse reverses the order of points *
 * in each polygon of a savg file               *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "feed.h"
#include "savgLineEvaluator.c"

#define pmax = 100;


#define MAX(x,y)  (((x)>(y))?(x):(y))


/* Prints information on usage*/
void usage();


int main(argc, argv)
int argc;
char** argv;
{
/*
 * INITIALIZATION
 */
  char line[STRING_LENGTH];
  char polyline[100][STRING_LENGTH];
  char templine[STRING_LENGTH];
  char assembleNewLine[STRING_LENGTH] = "";
  char tempString[20] = ""; 
  int i;
  int pargc = 0;
  char **pargv  = NULL;
  int cnt = 0;
  double tmpx = 0, tmpy = 0, tmpz = 0;
  int flip = 1;

/*
 * COMMAND LINE PROCESSING
 */
	if (argc > 2) {
	  fprintf (stderr, "savg-reverse: Bad argument count.\n");
	  usage();
	  exit(EXIT_FAILURE);
	}

	if (argc == 2)
		{
		int len = strlen (argv[1]);
		if ( (len > 1) && 
			(strncasecmp ("-noflip", argv[1], MAX (2, len)) == 0) )
			{
			flip = 0;
			}
		else
			{
			fprintf (stderr, 
			  "savg-reverse: Bad command line argument: \"%s\".\n",
				argv[1]);
			usage ();
			exit (EXIT_FAILURE);
			}
		}
			


/*
 * BODY
 */
	if(!getLine(line))
	  return 0;
	
        // jump to the nearest key word while not at EOF
	while(jumpToNearestPolygon(line,pargv,pargc))
	{
	  // print the polygon line
	  printf("%s\n", line);
	  if( !getLine(line) )
	    return 0;
	  cnt = 0;
	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      dpf_charsToArgcArgv(line, &pargc, &pargv);

	      // see if there are normals to flip
	      if (flip && (pargc == 6 || pargc == 10)) {
		// put together the xzy and rgba if there
		strcpy(assembleNewLine, pargv[0]);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[1]);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[2]);
		strcat(assembleNewLine, " ");

		if (pargc > 6) {
		  strcat(assembleNewLine, pargv[3]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[4]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[5]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[6]);
		  strcat(assembleNewLine, " ");
		}

		// find the normals and flip the sign
		tmpx = atof(pargv[pargc-3]); 
		tmpy = atof(pargv[pargc-2]);
		tmpz = atof(pargv[pargc-1]);
		sprintf(tempString, "%f", -tmpx);
		strcat(assembleNewLine, tempString);
		strcat(assembleNewLine, " ");
		sprintf(tempString, "%f", -tmpy);
		strcat(assembleNewLine, tempString);
		strcat(assembleNewLine, " ");
		sprintf(tempString, "%f", -tmpz);
		strcat(assembleNewLine, tempString);
		strcpy(polyline[cnt], assembleNewLine);

	      }
	      else {
		// copy line to temp, increment counter
		strcpy(polyline[cnt], line);
		//printf("print string: %s\n",polyline[cnt]);
	      }
	      ++cnt;
	    }
	    if(!getLine(line)) {
	      // print the lines in reverse
	      for (i = cnt-1; i >=0; --i) {
		printf("%s\n",polyline[i]);
	      }
	      return 0;
	    }
	  }
	  // print the lines in reverse
	  for (i = cnt-1; i >=0; --i) {
	    printf("%s\n",polyline[i]);
	  }
	}
        return 0;
}

/* Prints information on usage*/
void usage()
{
	fprintf(stderr,
	"Usage:\n"
	"\tsavg-reverse [-noFlip] \n");
}
