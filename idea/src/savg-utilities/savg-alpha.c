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
 * savg-alpha   				*
 * Created by: Adele Peskin			* 
 * Created on: 2-18-05				*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-alpha changes only the transparency	*
 * (alpha) in a savg file.  It takes 1          *
 * parameter, -a, which is associated with a    *
 * decimal value between 0.0 and 1.0.    	*
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "savgLineEvaluator.c"

/* Prints information on usage*/
void usage();
void printNoRgba();

int main(argc, argv)
int argc;
char *argv[];
{
/*
 * INITIALIZATION
 */
	double a = 1; /* Inital alpha*/

	char line[STRING_LENGTH];	/* holds the line from the savg file to be processed */ 
	int pargc = 0;
	char **pargv  = NULL;
	char assembleNewLine[STRING_LENGTH] = "";
	char tempString[20] = "";
	int isTextured = 0;
	int textureDim = 0;
        int next = 0;
	float ta, tb;
	char templine[STRING_LENGTH];
	char tempval[10];
/*
 * COMMAND LINE PROCESSING
 */
        // If there are no command line arguments, then color is white
	if( argc != 2 ) {
	  usage();
	  exit(EXIT_FAILURE);
	}
	
	a = atof(argv[1]);
	
        /* Check to make sure the value is valid */
	if( a > 1.00000001 | a < -0.00000001 ) {	
	  usage();
	  exit(EXIT_FAILURE);
	}

	// define a as a string to write out to file
	sprintf(tempString, "%f", a);
	
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
	while(jumpToNearestKeyWord(line,pargv,pargc))
	{
	  // see how many parameters to know if rgba info is on keyword line
	  dpf_charsToArgcArgv(line, &pargc, &pargv);

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
	    // update a text line if it has rgb info
	    updateTextString(templine, pargc, pargv, 7, a, ta, tb, tempval);
		
	    printf("%s\n", templine);
	  }

	  else if (pargc == 5) {
	    // print the keyword
	    strcpy(assembleNewLine, pargv[0]);
	    strcat(assembleNewLine, " ");

	    // print r,g,b
	    strcat(assembleNewLine, pargv[1]);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, pargv[2]);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, pargv[3]);
	    strcat(assembleNewLine, " ");

	    // print new a value
	    strcat(assembleNewLine, tempString);
	    
	    // print the assembled line
	    printf("%s\n", assembleNewLine);
	    isTextured = 0;
          }
	  else if (pargc == 6) {
	    //printf("printing keywords: %d\n",isTextured);
	    // print 2 keywords
	    strcpy(assembleNewLine, pargv[0]);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, pargv[1]);
	    strcat(assembleNewLine, " ");

	    // print r,g,b
	    strcat(assembleNewLine, pargv[2]);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, pargv[3]);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, pargv[4]);
	    strcat(assembleNewLine, " ");

	    // print new a value
	    strcat(assembleNewLine, tempString);
	    
	    // print the assembled line
	    printf("%s\n", assembleNewLine);
          }
	  else if (pargc == 3 && strcasecmp("TEXT",pargv[0])==0 &&
		   strcasecmp("string",pargv[1])==0) {
	    printf("%s\n", line);
	    // print an extra rgba defining line after the string
	    printf("TEXT RGBA 1 1 1 %s\n",tempString);
	    isTextured = 0;
	  }
	  else
	  {
	    // the line containing a key word was found not to
	    // contain RGBA info, so print it and go to the 
	    // next line.
	    printf("%s\n", line);
	    isTextured = 0;
	  }
	  if( !getLine(line) )
	    return 0;

	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      // see whats in the line and then update the alpha value
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
		  // case 1: only xyz uv data: add rgba
		  strcat(assembleNewLine, "1 1 1");
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		}
		else if (pargc == 7 || pargc == 9) {
		  // case 2: xyz rgba
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		  strcat(assembleNewLine, " ");
		  next = next + 3;
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempString);
		}
		else if (pargc == 6 || pargc == 8) { 
		  // case 3: xyz normals
		  // new rgba
		  strcat(assembleNewLine, "1 1 1");
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		  // normals
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		  strcat(assembleNewLine, " "); 
		}
		else if (pargc == 10 || pargc == 12) { 
		  // case 4: xyz rgba normals
		  // print rgba
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		  next = next + 4;
		  
		  // normals
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		}
		printf("%s\n", assembleNewLine);
	      }
	    
	      else {
		// this is text: see if its a rgba line
		if (strcasecmp(pargv[1],"RGBA")==0) {
		  strcpy(assembleNewLine, pargv[0]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[2]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[3]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[4]);
		  strcat(assembleNewLine, " ");
	    
		  // print new a value
		  strcat(assembleNewLine, tempString);
		  strcat(assembleNewLine, " ");
		  printf("%s\n", assembleNewLine);
		}
		else
		  printf("%s\n",line);
	      }
	    }
	           
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
	"\tsavg-alpha value\n"
	"\tvalue must be a decimal between 0.0 and 1.0\n");
}

void printNoRgba()
/*
 * Prints error message if rgb missing
 */
{
  fprintf(stderr,
	  "ERROR: Cannot update alpha: no rgb information.\n");
}
