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
 * savg-color					*
 * Created by: Brandon M. Smith			* 
 * Original "gtb" file by: Harry Bullen		*
 * Created on: 6-9-04				*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-color adds color and transparencey	*
 * (alpha) to a savg file.  It takes up to four	*
 * command line arguments, -r, -g, -b, and -a.	*
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
	int i; /* counter */
	char line[STRING_LENGTH];	/* holds the line from the savg file to be processed */ 
	int pargc = 0;
	char **pargv  = NULL;
	char assembleNewLine[STRING_LENGTH] = "";
	char tempStringr[20] = "";
	char tempStringg[20] = "";
	char tempStringb[20] = "";
	char tempStringa[20] = "";
	char templine[STRING_LENGTH];
	char tempval[10];
/*
 * COMMAND LINE PROCESSING
 */
        // If there are no command line arguments, then color is white
	if( 1 == argc )
		r = g = b = 1;

	for( i = 1; i < argc; i++)
	{
	    if(argc > (i+1))
	    {
		if(strcmp(argv[i], "-r") == 0)	
			r = atof(argv[++i]);
		else if(strcmp(argv[i], "-g") == 0)
			g = atof(argv[++i]);
		else if(strcmp(argv[i], "-b") == 0)
			b = atof(argv[++i]);
		else if(strcmp(argv[i], "-a") == 0)
			a = atof(argv[++i]);
		else 
		{ 
			usage();
			exit(EXIT_FAILURE);
		}
	    }
            else
	    {
	      usage();
	      exit(EXIT_FAILURE);
	    }
	}


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
	while(jumpToNearestKeyWord(line,pargv,pargc)) {
	  // see how many parameters to know if rgba info is on keyword line
	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  if (pargc == 5 )
	  {
	    // print the keyword
	    strcpy(assembleNewLine, pargv[0]);
	    strcat(assembleNewLine, " ");

	    // replace the rgb
	    strcat(assembleNewLine, tempStringr);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, tempStringg);
	    strcat(assembleNewLine, " ");
	    strcat(assembleNewLine, tempStringb);
	    strcat(assembleNewLine, " ");

	    // print the old a value
	    strcat(assembleNewLine, tempStringa);
	  	
    
	    // print the assembled line
	    printf("%s\n", assembleNewLine);
          }
	  else if (pargc > 2 && strcasecmp("TEXT",pargv[0])==0 &&
		   strcasecmp("string",pargv[1])==0) {
	    // update a text line if it has rgb info
	    updateTextString(templine, pargc, pargv, 5, r, g, b, tempval);
		
	    printf("%s\n", templine);
	  }
	  else
	    printf("%s\n", line);

	  if( !getLine(line) )
	    return 0;

	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      // see whats in the line and then update the alpha value
	      dpf_charsToArgcArgv(line, &pargc, &pargv);

	      // in any case, print xyz
	      strcpy(assembleNewLine, pargv[0]);
	      strcat(assembleNewLine, " ");
	      strcat(assembleNewLine, pargv[1]);
	      strcat(assembleNewLine, " ");
	      strcat(assembleNewLine, pargv[2]);
	      strcat(assembleNewLine, " ");
	      
	      // print the new rgba values
	      strcat(assembleNewLine, tempStringr);
	      strcat(assembleNewLine, " ");
	      strcat(assembleNewLine, tempStringg);
	      strcat(assembleNewLine, " ");
	      strcat(assembleNewLine, tempStringb);
	      strcat(assembleNewLine, " ");
	      strcat(assembleNewLine, tempStringa);
	      
	      // print nxyz if there
	      if (pargc == 6 || pargc == 10) {
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[pargc-3]);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[pargc-2]);
		strcat(assembleNewLine, " ");
		strcat(assembleNewLine, pargv[pargc-1]);
		strcat(assembleNewLine, " ");
	      }

	      printf("%s\n", assembleNewLine);
	    }
	    else
	      printf("%s\n", line);
	      
	      
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
	  "\tsavg-color [-r value | -g value | -b value | -a value]\n"
	  "\tvalue must be a decimal between 0.0 and 1.0\n");
}
