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
 * savg-rm-rgba 			      	*
 * Created by: Adele Peskin     		* 
 * Created on: 3-31-05  			*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-rm-rgba removes rgba values          *
 * from all lines that contain them.            *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "savgLineEvaluator.c"

/* Prints information on usage*/
void usage();

int main(argc, argv)
int argc;
char *argv[];
{
/*
 * INITIALIZATION
 */
	char line[STRING_LENGTH]; 
	int pargc = 0;
	char **pargv  = NULL;
	char assembleNewLine[STRING_LENGTH] = "";
	int isTextured = 0;
	int textureDim = 0;
	int next = 0;

/*
 * COMMAND LINE PROCESSING
 */
        // There are no command line arguments
	if( argc > 1) {
	  usage();
	  exit(EXIT_FAILURE);
	}

/*
 * BODY
 */   
	/*
         * getLine returns 1 if successful (ie not at
         * EOF) and 0 otherwise.  It is necessary to do
         * this test so that a "segmentation fault" is
	 * not encountered.
	 */
	if( !getLine(line) )
	  return 0;

        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line, pargv, pargc))
	{
	  // print the line unless it has rgba info on it
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

	  if ((pargc == 5 || pargc == 6) && 
	      keyWordIsAShape(pargv[0]))
	    printf("%s\n", pargv[0]);
	  else if (pargc == 3 && strcasecmp("TEXT",pargv[0])==0 &&
		   strcasecmp("RGBA",pargv[1])==0) {
	    // print nothing
	    isTextured = 0;
	  }
	  else
	    printf("%s\n", line);

	  if( !getLine(line) )
	    return 0;

	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      // see how many parameters to know what type of line
	      dpf_charsToArgcArgv(line, &pargc, &pargv);

	      next = 0;
	      // if not text data
	      if (strcasecmp(pargv[0],"text")!=0) {
		// for all cases, print x,y,z
		strcpy(assembleNewLine, pargv[0]);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[1]);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[2]);
		strcat(assembleNewLine, " ");
		next = 3;

		// if textured, print u,v,(w)
		if (isTextured) {
		  //printf("print uv\n");
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

		if (pargc == 3 || pargc == 5) {
		  // case 1: just xyz uv there: done
		}
		else if (pargc == 7 || pargc == 9) {
		  // case 2: xyz rgba: done
		}
		else if (pargc == 6 || pargc == 8) {
		  // case 3: xyz normals: print normals
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		  strcat(assembleNewLine, " ");
		}
		else if (pargc == 10 || pargc == 12) { 
		  // case 4: xyz rgba normals: print normals
		  next = next + 4;
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		  strcat(assembleNewLine, " "); 
		}
		printf("%s\n", assembleNewLine);

	      }

	      else {
		// this is text: see if its a rgba line
		if (strcasecmp(pargv[1],"RGBA")!=0)
		  printf("%s\n",line);
	      }
	    }
	    else
	      printf("%s\n",line);
	    
	    if(!getLine(line))
	      return 0;
	  }
	}
	return 0;
}	


void usage()
/*
 * Prints information on usage
 */
{
	fprintf(stderr,
	"ERROR: Incorrect command line arguments\n"
	"\n"
	"USAGE:\n"
	"\tsavg-rm-normals\n");
}
