/*
 * 
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
 * savg-split				        *
 * Created by: Adele Peskin     		*
 * Created on: 4-07-05  			*
 * Current Version: 1.00			*
 *						*
 * Description:	                                *
 *    savg-split splits the geometry in a large *
 * savg file into 2 files, leaving the keyword  *
 * lines in both.                               *
 * It outputs 2 files named by the parent file  *
 * file name plus "child1" or "child2".         *
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
  char child1name[STRING_LENGTH];
  char child2name[STRING_LENGTH];
  FILE* fp2;
  FILE* fp3;
  int filenumber = 1;
  int pargc = 0;
  char **pargv  = NULL;
/*
 * COMMAND LINE PROCESSING
 */
   if( argc != 3)
     usage();

   strcpy(child1name, argv[1]);
   strcpy(child2name, argv[2]);

   /*
    * BODY
    */ 
   // open up 2 output files
   fp2 = fopen(child1name, "w");
   fp3 = fopen(child2name, "w");

   // read the incoming file
   while (getLine(line)) {

     // parse each line and reprint to either one or both new files
     dpf_charsToArgcArgv(line, &pargc, &pargv);

     // if it's a blank write to both files
     if (pargc == 0) {
       fprintf(fp2, "%s\n",line);
       fprintf(fp3, "%s\n",line);
     }

     // if it's a comment, write to both
     else if (pargv[0][0] == '#') {
       fprintf(fp2, "%s\n",line);
       fprintf(fp3, "%s\n",line);
     }

     else {
       // is first word a keyword
       if (isKeyWordTest(pargv[0])) {
	 // if a geometry keyword, print to 1, if not to both
	 if(keyWordIsAShape(pargv[0])) {
	   // turn on writing to 1 file
	   // increment the file number
	   if (filenumber == 0)
	     filenumber = 1;
	   else
	     filenumber = 0;
	   if (filenumber == 0)
	     fprintf(fp2, "%s\n",line);
	   else
	     fprintf(fp3, "%s\n",line);
	 }
       }

       else {
	 // is it a data line?
	 if( pargc == 3 || pargc == 6 || pargc == 7 || pargc == 10 ) {
	   if (filenumber == 0)
	     fprintf(fp2, "%s\n",line);
	   else
	     fprintf(fp3, "%s\n",line);
	 }
       } 
     }
   }

   fclose (fp2);
   fclose (fp3); 

   return 0;
   
}

/* Prints information on usage*/
void usage()
{
  printf("usage: savg-split child1.savg child2.savg\n");
  exit(1);
}




