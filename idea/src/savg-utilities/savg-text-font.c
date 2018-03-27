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
 * savg-text-justification          			*
 * Created by: Adele Peskin		        *
 * Created on: 6-29-05				*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-text-justification left, center, or  *
 * right-justifies all text in a file.          *
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
  char line[STRING_LENGTH];
  char templine[STRING_LENGTH];
  char fontname[STRING_LENGTH];
  int pargc = 0;
  char **pargv  = NULL;
  float tmpx, tmpy, tmpz;
/*
 * COMMAND LINE PROCESSING
 */
  // make sure there is one argument
  if (argc != 2 ) {
    usage();
    exit(EXIT_FAILURE);
  }

  // the parameter is name of the font
  strcpy(fontname,argv[1]);

/*
 * BODY
 */


  // print the font at the top of the file
  printf("TEXT FONT %s\n",fontname);

  if( !getLine(line) )
    return 0;

  // jump to the nearest key word while not at EOF
  while(jumpToNearestKeyWord(line,pargv,pargc)) {
    // see how many parameters to know if rgba info is on keyword line
    dpf_charsToArgcArgv(line, &pargc, &pargv);
    
    if (pargc > 1 && strcasecmp("FONT",pargv[1])==0) { 
      // do not print
    }
    else if (pargc > 2 && strcasecmp("TEXT",pargv[0])==0 &&
	     strcasecmp("string",pargv[1])==0) {
      // update a text line if it has rgb info
      updateTextString(templine, pargc, pargv, 6, 
		       tmpx, tmpy, tmpz, fontname);
      
      printf("%s\n", templine);
    }
    else
      printf("%s\n", line);{
    }
	    
	    if(!getLine(line))
	      return 0;
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
	"\tsavg-text-font fontname\n");
}
