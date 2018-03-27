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
 * savg-rgb					*
 * Created by: Adele Peskin			* 
 * Created on: 3-02-05   			*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-rgb adds color to a savg file.       *
 * It takes 3 line arguments, -r, -g, -b.	*
 * Each is associated with a decimal value	*
 * between 0.0 and 1.0.  If an invalid command	*
 * line argument or value is given then an error*
 * message is printed and the program terminates*
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
	double r = 0, g = 0, b = 0, a = 1; /* Inital color & alpha*/	
	char line[STRING_LENGTH];	/* holds the line from the savg file to be processed */  
	int pargc = 0;
	char **pargv  = NULL;
	char assembleNewLine[STRING_LENGTH] = "";
	char tempStringr[20] = "";
	char tempStringg[20] = "";
	char tempStringb[20] = "";
	char tempStringa[20] = "";
	int isTextured = 0;
	int textureDim = 0;
	int next = 0;
	char templine[STRING_LENGTH];
	char tempval[10];

/*
 * COMMAND LINE PROCESSING
 */
	// there must be 4 arguments, savg-rgb r g b
	
	if( 4 != argc ) {	
	  usage();
	  exit(EXIT_FAILURE);
	} 

	r = atof(argv[1]);
	g = atof(argv[2]);
	b = atof(argv[3]);

        /* Check to make sure all values are valid */
	if( r > 1 | g > 1 | b > 1 | a > 1 | r < 0 | g < 0 | b < 0 | a < 0 )
	{	
	  usage();
	  exit(EXIT_FAILURE);
	}

	// make rgba into strings
	sprintf(tempStringr, "%f", r);
	sprintf(tempStringg, "%f", g);
	sprintf(tempStringb, "%f", b);
	sprintf(tempStringa, "%f", a);
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
	while(jumpToNearestKeyWord(line, pargv, pargc)) {
	  // see how many parameters to know if rgba info is on keyword line
	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  isTextured = 0;

	  if (pargc > 1) {
	    if (strcasecmp("2DTEXTURE",pargv[1])==0) {
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
	    updateTextString(templine, pargc, pargv, 5, r, g, b, tempval);
		
	    printf("%s\n", templine);
	  }
	  else if (pargc == 5 || pargc == 6)
	  {
	    // print the keyword
	    strcpy(assembleNewLine, pargv[0]);
	    strcat(assembleNewLine, " ");
	    if (pargc == 6) {
	      // print texture key word
	      strcat(assembleNewLine, pargv[1]);
	      strcat(assembleNewLine, " ");
	    }

	    // replace the rgb
	    strcat(assembleNewLine, tempStringr);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, tempStringg);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, tempStringb);
	    strcat(assembleNewLine, " ");

	    // print the old a value
	    if (pargc == 5)
	      strcat(assembleNewLine, pargv[4]);
	    else
	      strcat(assembleNewLine, pargv[5]);

	    // print the assembled line
	    printf("%s\n", assembleNewLine);
	    if (pargc == 5)
	      isTextured = 0;
          }
	  else
	    printf("%s\n", line);

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

		// print the new rgb values
		strcat(assembleNewLine, tempStringr);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, tempStringg);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, tempStringb);
		strcat(assembleNewLine, " ");

		if (pargc == 3 || pargc == 5) {
		  // case 1: just xyz uv there: print alpha = 1;
		  strcat(assembleNewLine, tempStringa);
		  strcat(assembleNewLine, " ");
		}
		else if (pargc == 7 || pargc == 9) {
		  // case 2: xyz rgba: print old alpha value
		  next = next + 3;
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		}
		else if (pargc == 6 || pargc == 8) { 
		  // case 3: xyz normals 
		  // print alpha = 1
		  strcat(assembleNewLine, "1");
		  strcat(assembleNewLine, " ");
		  // print normals
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+1]);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, pargv[next+2]);
		  strcat(assembleNewLine, " ");
		}
		else if (pargc == 10 || pargc == 12) { 
		  // case 4: xyz rgba normals: print old alpha value
		  next = next + 3;
		  strcat(assembleNewLine, pargv[next]);
		  strcat(assembleNewLine, " ");
		  next = next + 1;
		  // print normals
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

		  strcat(assembleNewLine, tempStringr);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempStringg);
		  strcat(assembleNewLine, " ");
		  strcat(assembleNewLine, tempStringb);
		  strcat(assembleNewLine, " ");

		  strcat(assembleNewLine, pargv[5]);
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
	"\tsavg-rgb r-value g-value b-value\n"
	"\tvalue must be a decimal between 0.0 and 1.0\n");
}
