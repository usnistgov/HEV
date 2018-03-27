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


/***************************************************
 * savg-scale		  			   *
 * Created by: Brandon M. Smith                    *
 * Original "gtb" file created by: Harry Bullen	   *
 * Created on: 6-17-04				   *
 * Current Version: 1.00			   *
 *						   *
 * Description:					   *
 *    savg-scale scales savg files acording to its *
 * command line arguments, -x, -y, and -z.  If	   *
 * no arguments are given, a usage message is      *
 * printed, as without command line arguments	   *
 * savg-scale can not do anything.  Also all	   *
 * comments in the files are left intact.	   *
 **************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "feed.h"
#include "savgLineEvaluator.c"
#define minValue 0.00000001

/* Prints information on usage*/
void usage();

int main(argc, argv)
int argc;
char** argv;
{
/*
 * INITIALIZATION
 */

	double xScale = 1, yScale = 1, zScale = 1; /* scale values */
	double xvNew, yvNew, zvNew; /* storage for vertices */
	double xnNew, ynNew, znNew; /* storage for the normal */
	double vectorMagnitude;
	
	char line[STRING_LENGTH]; 
	int pargc = 0;
	char **pargv  = NULL;
	char assembleNewLine[STRING_LENGTH] = "";
	char tempString[20] = "";
	int isTextured = 0;
	int textureDim = 0;
	int next = 0;
	char templine[STRING_LENGTH];
	char tempval[10];
/*
 * COMMAND LINE PROCESSING
 */
	if (argc < 2) {
	  usage();
	  exit(EXIT_FAILURE);
	}

	if (argc == 2) {
	  xScale = atof(argv[1]);
	  yScale = xScale;
	  zScale = xScale;
	}

	else if (argc == 4) {
	  xScale = atof(argv[1]);
	  yScale = atof(argv[2]);
	  zScale = atof(argv[3]);
	}

	else {
	  usage();
	  exit(EXIT_FAILURE);
	}

/*
 * BODY
 */

	if(!getLine(line))
	  return 0;
	
        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line, pargv, pargc)) {

	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  isTextured = 0;

	  if (pargc > 1) {
	    if (strcasecmp("2DTEXTURE",pargv[1])==0) {
	      //printf("got istextured\n");
	      isTextured = 1;
	      textureDim = 2;
	    }
	    if (strcasecmp("3DTEXTURE",pargv[1])==0) {
	      isTextured = 1;
	      textureDim = 3;
	    }
	  }
	  if (pargc > 2 && strcasecmp("TEXT",pargv[0])==0 &&
		   strcasecmp("string",pargv[1])==0) {
	    if (argc == 2)
	      updateTextString(templine, pargc, pargv, 3,
				 xScale, yScale, zScale,tempval);
	      else
		updateTextString(templine, pargc, pargv, 4,
				 xScale, yScale, zScale,tempval);
		
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
		// in any case, update and then print xyz	  
		xvNew = xScale * atof(pargv[0]);
		yvNew = yScale * atof(pargv[1]);
		zvNew = zScale * atof(pargv[2]);
		sprintf(tempString, "%f", xvNew);
		strcpy(assembleNewLine, tempString);
		strcat(assembleNewLine, " ");
		sprintf(tempString, "%f", yvNew);
		strcat(assembleNewLine, tempString);
		strcat(assembleNewLine, " ");
		sprintf(tempString, "%f", zvNew);
		strcat(assembleNewLine, tempString);
		strcat(assembleNewLine, " ");
		next = 3;
		
		// print uv if appropriate
		if (isTextured) {
		  strcat(assembleNewLine, pargv[3]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[4]);
		  strcat(assembleNewLine, " ");
		  next = 5;
		  if (textureDim == 3) {
		    strcat(assembleNewLine, pargv[5]);
		    strcat(assembleNewLine, " ");
		    next = 6;
		  }
		}	
	
		// print the rgba values if present
		if (pargc == 7 || pargc == 10 ||
		    pargc == 9 || pargc == 12) {
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+3]);
		  strcat(assembleNewLine, " ");
		  next = next + 4;
		  strcat(assembleNewLine, " ");
		}

		// update and print normals if there
		if (pargc == 6 || pargc == 10 ||
		    pargc == 8 || pargc == 12) {
		  // update the normals
		  xnNew = atof(pargv[pargc-3]) * yScale * zScale;
		  ynNew = atof(pargv[pargc-2]) * xScale * zScale;
		  znNew = atof(pargv[pargc-1]) * xScale * yScale;
		  vectorMagnitude = sqrt( xnNew*xnNew + ynNew*ynNew + znNew*znNew );
		  if(fabs(vectorMagnitude) > minValue) {
		    xnNew = xnNew / vectorMagnitude;
		    ynNew = ynNew / vectorMagnitude;
		    znNew = znNew / vectorMagnitude;
		  }
		  sprintf(tempString, "%f", xnNew);
		  strcat(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		  sprintf(tempString, "%f", ynNew);
		  strcat(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		  sprintf(tempString, "%f", znNew);
		  strcat(assembleNewLine, tempString);
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
	"\tsavg-scale [value] or [xvalue | yvalue | zvalue]\n"
	"\tvalues are required and are floating points.\n");
}
