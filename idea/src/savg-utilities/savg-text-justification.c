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
  int just = 0; 
  int pargc = 0;
  char **pargv  = NULL;

/*
 * COMMAND LINE PROCESSING
 */
  // make sure there is one argument
  if (argc != 2 ) {
    usage();
    exit(EXIT_FAILURE);
  }

  if (strcasecmp(argv[1],"LEFT")==0)
    just = 1;
  else if (strcasecmp(argv[1],"CENTER")==0)
    just = 2;
  else if (strcasecmp(argv[1],"RIGHT")==0)
    just = 3;
  else {
    usage();
    exit(EXIT_FAILURE);
  }

/*
 * BODY
 */

  // print the text justification at the top of the file
  if (just == 1)
    printf("TEXT JUSTIFICATION LEFT\n");
  if (just == 2)
    printf("TEXT JUSTIFICATION CENTER\n");
  if (just == 3)
    printf("TEXT JUSTIFICATION RIGHT\n");
 
  // remove any other text justification lines 
  while(getLine(line)) {

    // if it is a comment just print it
    if (containsComment(line))
      printf("%s\n",line);

    else {
      // parse and check each line
      dpf_charsToArgcArgv(line, &pargc, &pargv);

      if (pargc == 0)
	printf("\n");
      else if (pargc == 1)
	printf("%s\n",line);
      else if (pargc > 1 && strcasecmp("JUSTIFICATION",pargv[1])!=0)
	  printf("%s\n",line);
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
	"\tsavg-text-justification LEFT/CENTER/RIGHT\n");
}
