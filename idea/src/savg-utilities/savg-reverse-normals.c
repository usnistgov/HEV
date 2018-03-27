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


/****************************************************
 * savg-reverse-normals				    *
 * Created by:                                      * 
 * Created on: January 2009                         *
 * Current Version: 1.00			    *
 *						    *
 * Description:					    *
 *    savg-reverse-normals reverses the direction   *
 *    of all normals.                               *
 ****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "savgLineEvaluator.c"

/* Prints information on usage*/
static void 
usage()
	{
	fprintf (stderr, "\n");
	fprintf (stderr, "Usage: savg-reverse-normals\n");
	fprintf (stderr, "          Reverses the direction of all normals.\n");
        fprintf (stderr, "          No arguments; no options.\n");
	fprintf (stderr, "\n");
	}  // end of usage

//
// negateStr - adds or removes a negative sign to a token.
//             This is a last resort if it can't be done numerically.
//
static void
negateStr (char *inNormStr, char *negNormStr)
	{
	int len, wLen;

	len = strlen (inNormStr);

	// find initial white space and get rid of it.
	wLen = strspn (inNormStr, " \t\n\v\f\r");
	inNormStr += wLen;

	len = strlen (inNormStr);

	if (len == 0)
		{
		// this is bad and should not happen
		strcpy (negNormStr, "0");
		}
	else
		{
		if (inNormStr[0] == '-')
			{
			if (len == 1)
				{
				// token was just "-"
				// this is bad and should not happen
				strcpy (negNormStr, "0");
				}
			else
				{
				// if it starts with "-", just remove it.
				strcpy (negNormStr, inNormStr+1);
				}
			}
		else
			{
			// if it doesn't start with "-", then add it.
			strcpy (negNormStr, "-");
			strcat (negNormStr, inNormStr);
			}
		}

	return;
	}  // end of negateStr

static void
printBadLinesMsg (int badNormalValue, int badTokenCount)
	{
	if (badNormalValue || badTokenCount)
		{
		fprintf (stderr, 
		    		"savg-reverse-normals: "
		    		"Some bad input data lines were found:\n");
		if (badNormalValue)
			{
			fprintf (stderr, 
				"        "
				"Ill-formed normal value(s) found.\n");
			}
		if (badTokenCount)
			{
			fprintf (stderr, 
				"        "
				"Invalid number of values found.\n");
			}
		fprintf (stderr, 
	    		"savg-reverse-normals: "
	    		"See output data for comments describing errors.\n");
		}

	}  // end of printBadLinesMsg

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
	int badLinesFound = 0;
	int badNormalValue = 0;
	int badNormalValueCurrLine = 0;
	int i;
	double norm;
	int normalStart, normalPresent;
	int coordLen, rgbLen;
	int badLine = 0;
	int badTokenCount = 0;

/*
 * COMMAND LINE PROCESSING
 */
        // There are no command line arguments
	if( argc > 1) {
	  fprintf (stderr, "savg-reverse-normals: Bad argument count.\n");
	  usage();
	  exit(EXIT_FAILURE);
	}

/*
 * BODY
 */   
	/*
         * getLine returns 1 if successful (ie not at EOF) and 0 otherwise.  
	 */
	if( !getLine(line) )
	  return 0;

        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line, pargv, pargc)) 
	{


	  // see how many parameters to know if rgba info is on keyword line
	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  isTextured = 0;
	  textureDim = 0;

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
	  printf("%s\n", line);

	  if( !getLine(line) )
	    return 0;
	  
	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) 
	    {

	    if(!isBlankLine(line)) 
		{

		// tokenize the line
		dpf_charsToArgcArgv(line, &pargc, &pargv);



		coordLen = 3 + textureDim;
		rgbLen = coordLen + 4;

		normalPresent = 0;
		badLine = 0;
		badNormalValueCurrLine = 0;
	
		if  ( (pargc == rgbLen) || (pargc == coordLen) )
			{
			normalPresent = 0;
			}
		else if (pargc == (rgbLen + 3)) 
			{
			normalPresent = 1;
			normalStart = rgbLen;
			}
		else if (pargc == (coordLen+3))
			{
			normalPresent = 1;
			normalStart = coordLen;
			}
		else
			{
			// something is wrong; write error msg?
			badTokenCount = 1;
			badLinesFound = 1;
			normalPresent = 0;
			badLine = 1;
			printf ("# savg-reverse-normals: "
				"Bad token count in input data on \n"
				"#   line corresponding to next output line.\n");
			}
			

		if ( badLine || (!normalPresent) )
			{
			printf ("%s\n", line);
			}
		else
			{
			// print out all of the tokens prior to the normal
			for (i = 0; i < normalStart; i++)
				{
				printf ("%s ", pargv[i]);
				}

			// negate each normal component and print it out
			for (i = normalStart; i < normalStart+3; i++)
				{

				if (sscanf (pargv[i], "%lf", &norm) != 1)
					{
					char negNormStr[1000];
					badLinesFound = 1;
					badNormalValue = 1;
					badNormalValueCurrLine = 1;
					negateStr (pargv[i], negNormStr);
					printf ("%s ", negNormStr);
					}
				else
					{
					if (norm != 0)  // try to avoid -0 
						{
						norm *= -1;
						}
					printf ("%g ", norm);
					}
				}

			// end the line
			printf ("\n");
			if (badNormalValueCurrLine)
				{
				printf ("# savg-reverse-normals: "
					  "Ill-formed normal found in "
					  "input data on \n"
					  "#   line corresponding to "
					  "previous output line.\n");
				badNormalValueCurrLine = 0;
				}

			}  // end of clause for handling norm negation


		}   // end of clause for non-blank lines)

		// get the next line
		if(!getLine(line))
			{
			if (badLinesFound)
				{
				printBadLinesMsg (badNormalValue, badTokenCount);
				}
			exit (0);
			}

		badNormalValueCurrLine;

	    }   // end of loop over data lines (and blank lines)

	}   // end of loop over sections of file initiated by keywords


	if (badLinesFound)
		{
		printBadLinesMsg (badNormalValue, badTokenCount);
		}

	exit (0);

}  // end of main	


