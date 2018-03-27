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
 * savg-add-normals				*
 *						*
 * Description:					*
 *    savg-add-normals adds normals to polygons.*
 *                                              *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "savgLineEvaluator.c"

#define MAX(x,y)  (((x)<(y))?(x):(y))

#define MAX_POLY_LINES (2000)


void usage()
{
	fprintf (stderr, "Usage:  savg-add-normals [-replace]\n");
}



static void
outputLineWithNorm (char *line, char *normStr, int replaceNorm)
	{
	int pargc = 0;
	char **pargv  = NULL;
	int badArgCount = 0;
	int normalPresent, normalStart;
	int i;
	
	// first we need to figure out if the normal is already present,
	// and if so, where to put it.
	dpf_charsToArgcArgv (line, &pargc, &pargv);

	if (pargc == 6)
		{
		normalPresent = 1;
		normalStart = 3;
		}
	else if (pargc == 10)
		{
		normalPresent = 1;
		normalStart = 7;
		}
	else if (pargc == 7)
		{
		normalPresent = 0;
		normalStart = 7;
		}
	else if (pargc == 3)
		{
		normalPresent = 0;
		normalStart = 3;
		}
	else
		{
		badArgCount = 1;
		}


	if (badArgCount)
		{
		// can't really parse the line, so just output it
  		printf("%s\n", line);
		}
	else
		{

		if (replaceNorm || (!normalPresent))
			{
			// we write out the data from the line plus
			// the new norm
			for (i = 0; i < normalStart; i++)
				{
				printf ("%s ", pargv[i]);
				}

			printf ("%s\n", normStr);
			}
		else
			{
			// If the normal is present and we are not supposed
			// to replace the norm, then we just write out the
			// original line.
			printf ("%s\n", line);
			}
		}

	return;
	}  // end of outputLineWithNorm


static int samePt (double a[3], double b[3])
	{
	return ( 
		(a[0] == b[0]) &&
		(a[1] == b[1]) &&
		(a[2] == b[2]) );
	}  // end of samePt

static double vLen (double v[3])
	{
	return sqrt ( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
	}

static void
normalize (double v[3])
	{
	double len = vLen (v);

	if (len != 0)
		{
		v[0] /= len;
		v[1] /= len;
		v[2] /= len;
		}

	return;
	}  // end of normalize

static void
outputPolyData 
  (
  int numDataLines, 
  char polyDataLines[MAX_POLY_LINES][STRING_LENGTH], 
  int replaceNorm,
  char *normStr
  )

	{


	// We look at the polygon data lines stored in the array 
	// and try to derive a normal for the polygon.

	// After deriving the normal, we write out the lines with
	// the newly calculated normal.

	double pt[3][3];
	double newPt[3], u[3], v[3], norm[3], dummy;
	double dot;
	int numPtsFound = 0;
	int i, j;
	static int iPoly = 0;
	int ptOK;

	iPoly ++;

	// first we need to find three distinct pts in polygon
	// that determine two non-colinear edges.

	for (i = 0; i < numDataLines; i++)
		{


		if (numPtsFound >= 3)
			{
			break;
			}

		if (sscanf (polyDataLines[i], "%lf %lf %lf", 
					newPt, newPt+1, newPt+2) == 3)
			{
			ptOK = 1;

			for (j = 0; j < numPtsFound; j++)
				{
				ptOK = ptOK && ( ! samePt (newPt, pt[j]));
				}

			if (ptOK)
				{
				pt[numPtsFound][0] = newPt[0];
				pt[numPtsFound][1] = newPt[1];
				pt[numPtsFound][2] = newPt[2];
				numPtsFound++;
				}
			}
		
		if (numPtsFound == 3)
			{

			// when we've found three distinct points, we
			// try to derive a normal using the cross product.

			u[0] = pt[0][0] - pt[1][0];
			u[1] = pt[0][1] - pt[1][1];
			u[2] = pt[0][2] - pt[1][2];

			v[0] = pt[1][0] - pt[2][0];
			v[1] = pt[1][1] - pt[2][1];
			v[2] = pt[1][2] - pt[2][2];

			normalize (u);
			normalize (v);

			dot = fabs (u[0]*v[0] + u[1]*v[1] + u[2]*v[2]);
			if (dot > 0.9994) // if angle is less than about 2 degrees
			    {
			    // if the dot product indicates that the angle
			    // between u and v is too small, we
			    // will look for a substitute for the third
			    // point.
			    numPtsFound--;
			    }

			else
			    {
			    calculateCrossProduct (u, v, norm, dummy);
			    if (vLen (norm) == 0)
				{
				// if the cross product is zero, then
				// the three points are colinear, so we
				// will look for a substitute for the third
				// point.
				// This branch will never be taken because
				// we've already done the colinearity test
				// above when we looked at the dot product.
				numPtsFound--;
				}
			    else
				{
				// we've found our non-colinear points, so
				// all we have to do is make the norm unit len
				normalize (norm);
				break;
				}

			    }
			}

		}   // end of loop over lines looking for norm


	if (numPtsFound < 3)
		{
		// if we've been unable to find the three distinct non-colinear
		// points, then we're out of luck!
		fprintf (stderr, 
		   "savg-add-normals: "
		   "Cannot calculate normal for polygon %d.\n", iPoly);
		fprintf (stderr, "                  Using normal 0 0 1.\n");
		norm[0] = norm[1] = 0;
		norm[2] = 1;
		}


	// We create a string that will output as the normal data
	sprintf (normStr, " %g %g %g", norm[0], norm[1], norm[2]);

	for (i = 0; i < numDataLines; i++)
		{
		// then output each line
		outputLineWithNorm ( polyDataLines[i], normStr, replaceNorm);
		}


	return;
	}  // end of outputPolyData





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
	int replaceNorms;
	char normStr[100];
	int normReady;
	int numDataLines;
	int atEOF;

	char polyDataLines[MAX_POLY_LINES][STRING_LENGTH];
/*
 * COMMAND LINE PROCESSING
 */
        // There are no command line arguments
	if( argc > 2) {
	  fprintf (stderr, "savg-add-normals: Bad argument count.\n");
	  usage();
	  exit(EXIT_FAILURE);
	}

	replaceNorms = 0;

	if (argc == 2)
		{
		int len = strlen (argv[1]);

		if (strncasecmp ("-replace", argv[1], MAX (len, 2)) == 0)
			{
			replaceNorms = 1;
			}
		else
			{
			fprintf (stderr, 
			  "savg-add-normals: "
			  "Bad command line argumement \"%s\".\n", argv[1]);
			usage ();
			exit (EXIT_FAILURE);
			}

		}


/*
 * BODY
 */   
	/* getLine returns 1 if successful (ie not at EOF) and 0 otherwise.  */
	if( !getLine(line) )
	  return 0;

	atEOF = 0;

        // jump to the nearest key word while not at EOF
	while((! atEOF) && jumpToNearestPolygon (line, pargv, pargc)) 
	  {   
	  
	  // print out the "polygon" line
	  printf("%s\n", line);

	  if( !getLine(line) )
	    return 0;
	  


	  // Now we loop through the data lines, storing each one
	  // until we either overflow the polyDataLines array or
	  // hit the end of the polygon.
	  numDataLines = 0;
	  normReady = 0;
	  while (isCorrectDataLine(line,pargv,pargc) || 
                 isBlankLine(line)) 
	    {
	    if(!isBlankLine(line)) 
		{

		// Note that normReady is true only after we have overflowed
		// the data line array and we have generated the norm.

		if (normReady)
			{
			outputLineWithNorm (line, normStr, replaceNorms);
			}
		else if (numDataLines < MAX_POLY_LINES)
			{
	      		strcpy (polyDataLines[numDataLines++], line);
			}
		else
			{
			outputPolyData (numDataLines, polyDataLines, 
						replaceNorms, normStr);
			normReady = 1;
			}
		}

	    if(!getLine(line))
		{
		atEOF = 1;
		break;
		}

	    }   // end of loop over data lines


	  if ( ! normReady )
		{
		// we get here when we haven't overflowed the polyDataLines 
		// array, so we haven't written out the polygon data.
		outputPolyData (numDataLines, polyDataLines, replaceNorms, 
				normStr);
		}


	  }   // end of loop over polygons

	return 0;
}  // end of main	


