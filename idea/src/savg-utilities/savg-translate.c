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

/* Prints information on usage*/
void usage();

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
	int ii;
	char tempval[10];

/*
 * COMMAND LINE PROCESSING
 */
	if (argc != 4) {
	  usage();
	  exit(EXIT_FAILURE);
	}
			
	x = atof(argv[1]);
	y = atof(argv[2]);
	z = atof(argv[3]);

/*
 * BODY
 */
	if(!getLine(line))
	  return 0;
	
        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line,pargv,pargc))
	{

	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  //printf("keyword line: %s pargc %d\n",line,pargc);

	  if (pargc > 2 && strcasecmp("TEXT",pargv[0])==0 &&
		   strcasecmp("string",pargv[1])==0) {
	    updateTextString(templine, pargc, pargv, 1, x, y, z, tempval);
	    printf("%s\n", templine);
	  }
	  else
	    printf("%s\n", line);
	  if( !getLine(line) )
	    return 0;

	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      dpf_charsToArgcArgv(line, &pargc, &pargv);
	      
	      if (strcasecmp(pargv[0],"text")==0) {
		printf("%s\n", line);
	      }

	      else {
		tmpx = x + atof(pargv[0]); 
		tmpy = y + atof(pargv[1]);
		tmpz = z + atof(pargv[2]);
		
		// print out the updated xyz values
		sprintf(tempString, "%f", tmpx);
		strcpy(assembleNewLine, tempString);
		strcat(assembleNewLine, " ");
		sprintf(tempString, "%f", tmpy);
		strcat(assembleNewLine, tempString);
		strcat(assembleNewLine, " ");
		sprintf(tempString, "%f", tmpz);
		strcat(assembleNewLine, tempString);
		strcat(assembleNewLine, " ");
		
		// copy the rest of the line
		for (ii = 3; ii < pargc; ++ii) {
		  strcat(assembleNewLine, pargv[ii]);
		  strcat(assembleNewLine, " ");
		}
		
		printf("%s\n", assembleNewLine);
	      }
	    }
	    if(!getLine(line))
	      return 0;
	  }
	}
        return 0;
}

/* Prints information on usage*/
void usage()
{
	fprintf(stderr,
	"ERROR: Incorrect command line arguments\n"
	"\n"
	"USAGE:\n"
	"\tsavg-translate [xvalue yvalue zvalue]\n"
	"\tvalues are required and are floating points.\n");
}
