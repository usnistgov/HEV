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
 * savg-wireframe		       		   *
 * Created by: Brandon M. Smith                    *
 * Created on: 6-21-04				   *
 * Current Version: 1.00			   *
 *						   *
 * Description:					   *
 *    savg-wireframe changes savg files            *
 * with filled in shapes into a wireframe with     *
 * lines connecting all the vertices.              *
 *                                                 *
 ***************************************************/

/*

  Note: Many of the calls with arguments pargv and pargc do not use 
        these arguments as you might expect.  Probably the only
        calls that use these arguments properly are the calls 
        to dpf_charsToArgcArgv. 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "feed.h"
#include "savgLineEvaluator.c"

#define ARRAY_CAPACITY (1000)

/* Prints information on usage*/
void usage();

/* Cuts a shape up into a series of unique lines */
int parseShape(char *pline, char **pargv, int pargc);

/* 
 * Stores a point if unique, otherwise returns
 * index of a matching point 
 */
int storePoint(double *pointVec3, double *RGBAVec4);

/*
 * Stores unique connectivities between points. If
 * sent a non-unique set of point, will return 1 meaning
 * "redundant".
 */
int storeConnectivity(int pointA, int pointB);


int getArgCount(char *pline)
/*
 * Returns the number of arguments in the string pline 
 */
{
  int argc;
  char **argv;
  dpf_charsToArgcArgv(pline, &argc, &argv);
  return argc;  
}


double getArgValueAsDouble(char *pline, int index)
/*
 * Returns the double value of the argument in pline that
 * corresponds to index.
 */
{
  int argc;
  char **argv;
  dpf_charsToArgcArgv(pline, &argc, &argv);
  
  return atof( argv[index] );
}


/*
 * Global variables
 */
int storeConnectEndIndex = 0;
int storePointEndIndex = 0;
double** pointStore;
int** connectivityStore;
int maxPoint = ARRAY_CAPACITY;
int maxConnect = ARRAY_CAPACITY;


int main(argc, argv)
int argc;
char** argv;
{
/*
 * INITIALIZATION
 */ 
  char line[STRING_LENGTH];
  int pargc = 0;
  char **pargv  = NULL;
  int isShape;
	
/*
 * COMMAND LINE PROCESSING
 */

if(argc > 1)
{
  usage();
  exit(EXIT_FAILURE);
}

/*
 * BODY
 */

   // assign some initial space for the point and connectivity arrays 
   pointStore = (double **) realloc(pointStore, sizeof(double*) * (ARRAY_CAPACITY));
   connectivityStore = (int **) realloc(connectivityStore, sizeof(int*) * (ARRAY_CAPACITY));

        printf("#NOTE: This file was altered using savg-wireframe.  \n#Some of the comments shown may no longer apply.\n\n");

        if(!getLine(line))
	/* getLine returns 0 if at end of file */
	  return 0; 
	
        if(!jumpToNearestKeyWord(line,pargv,pargc))
	/* jumpToNearestKeyWord returns 0 if at end of file */
	  return 0;

        while( !atEOF() )
	{

  	  dpf_charsToArgcArgv (line, &argc, &argv);

	  isShape = 0;
	  if (argc > 0)
		{
		isShape = keyWordIsAShape (argv[0]);
		}

	  if( isShape )
	  {


            if(!parseShape(line, pargv, pargc))
	    /* parseShape returns 0 if at end of file and 1 when end of shape is reached */
	      break;
	    if(!jumpToNearestKeyWord(line,pargv,pargc))
	      break;
          }
	  else 
	  /*
	   * non-shape keyword found so print it and move on
	   */
	  {
	    printf("%s\n", line);

	    if(!getLine(line))
	      break;
	    if(!jumpToNearestKeyWord(line,pargv,pargc))
	      break;
	  }
        }
	return 0;
}


int storePoint( double *pointVec3, double *RGBAVec4 )
{
  int index = 0;

  while(1)
  {
    if( index >= storePointEndIndex ) //end of storage
    {
      // new point - assign some space
      pointStore[storePointEndIndex] = (double*)malloc(sizeof(double)*7);

      pointStore[storePointEndIndex][0] = pointVec3[0];
      pointStore[storePointEndIndex][1] = pointVec3[1];
      pointStore[storePointEndIndex][2] = pointVec3[2];
      pointStore[storePointEndIndex][3] = RGBAVec4[0];
      pointStore[storePointEndIndex][4] = RGBAVec4[1];
      pointStore[storePointEndIndex][5] = RGBAVec4[2];
      pointStore[storePointEndIndex][6] = RGBAVec4[3];

      storePointEndIndex++;
      return index;
    }
    else if(pointStore[index][0] == pointVec3[0] &&
            pointStore[index][1] == pointVec3[1] &&
            pointStore[index][2] == pointVec3[2] &&
            pointStore[index][3] == RGBAVec4[0] &&
            pointStore[index][4] == RGBAVec4[1] &&
            pointStore[index][5] == RGBAVec4[2] &&
            pointStore[index][6] == RGBAVec4[3])
    {
      return index;
    }
    else
    {
      index++;
    }

    if( index >= maxPoint)
    {
      maxPoint += ARRAY_CAPACITY;
      //printf("new maxPoint: %d \n",maxPoint);
      pointStore = (double **) realloc(pointStore, sizeof(double)*(maxPoint));
    }
  }
}


int storeConnectivity( int pointA, int pointB )
{
  int index = 0;
  int temp;

  if( pointA > pointB )   
  {
    //swap values
    temp = pointA;
    pointA = pointB;
    pointB = temp;
  }

  while(1)
  {
    if( index >= storeConnectEndIndex ) //location at index is empty
    {
      // new connectivity - assign some space
      connectivityStore[storeConnectEndIndex] = (int*)malloc(sizeof(int)*2);
      connectivityStore[storeConnectEndIndex][0] = pointA;
      connectivityStore[storeConnectEndIndex][1] = pointB;
      storeConnectEndIndex++;
      return 0;  //not redundant line
    }
    else if( connectivityStore[index][0] == pointA &&
	     connectivityStore[index][1] == pointB )
    {
      return 1; //line is redundant
    }
    else
    {
      index++;
    }

    if( storeConnectEndIndex >= maxConnect )
    {
      maxConnect += ARRAY_CAPACITY;
      //printf("new maxConnect: %d \n",maxConnect);
      connectivityStore = (int **) realloc(connectivityStore, sizeof(int*)*(maxConnect) );
      return 0;
    }
  }
}


int parseShape(char *pline, char **pargv, int pargc)
{
  double fromPointVec3[3],
         firstPointVec3[3],
         toPointVec3[3],
         fromPointRGBAVec4[4],
         firstPointRGBAVec4[4],
         toPointRGBAVec4[4];
       
  int fromConnectivity, toConnectivity;
  int lineIsRedundant;
  int rgbaIsInKeyWordLine = 0;
  int atEndOfShape = 0;

  dpf_charsToArgcArgv(pline, &pargc, &pargv);
   

  if( pargc == 5 )
  // there are RGBA values associated with key word line
  {
    fromPointRGBAVec4[0] = firstPointRGBAVec4[0] = toPointRGBAVec4[0] 
                         = getArgValueAsDouble(pline, 1);
    fromPointRGBAVec4[1] = firstPointRGBAVec4[1] = toPointRGBAVec4[1] 
                         = getArgValueAsDouble(pline, 2);   
    fromPointRGBAVec4[2] = firstPointRGBAVec4[2] = toPointRGBAVec4[2] 
                         = getArgValueAsDouble(pline, 3);
    fromPointRGBAVec4[3] = firstPointRGBAVec4[3] = toPointRGBAVec4[3] 
                         = getArgValueAsDouble(pline, 4);

    rgbaIsInKeyWordLine = 1;
  }

  do
  {
    if(!getLine(pline))
      return 0;
  } while( !isCorrectDataLine(pline,pargv,pargc) );

  fromPointVec3[0] = firstPointVec3[0] = getArgValueAsDouble(pline, 0);
  fromPointVec3[1] = firstPointVec3[1] = getArgValueAsDouble(pline, 1);
  fromPointVec3[2] = firstPointVec3[2] = getArgValueAsDouble(pline, 2);

  if( !rgbaIsInKeyWordLine )
  {
    if( getArgCount(pline) >= 7 )
    {
      fromPointRGBAVec4[0] = firstPointRGBAVec4[0]
                           = getArgValueAsDouble(pline, 3);
      fromPointRGBAVec4[1] = firstPointRGBAVec4[1]
                           = getArgValueAsDouble(pline, 4);   
      fromPointRGBAVec4[2] = firstPointRGBAVec4[2]
                           = getArgValueAsDouble(pline, 5);
      fromPointRGBAVec4[3] = firstPointRGBAVec4[3] 
                           = getArgValueAsDouble(pline, 6);
    }
    else
    /* No color is indicated; make default opaque white */ 
    {
      fromPointRGBAVec4[0] = firstPointRGBAVec4[0] = 1.0;
      fromPointRGBAVec4[1] = firstPointRGBAVec4[1] = 1.0;
      fromPointRGBAVec4[2] = firstPointRGBAVec4[2] = 1.0;
      fromPointRGBAVec4[3] = firstPointRGBAVec4[3] = 1.0;
    }
  }

  do
  {
    if(!getLine(pline))
      return 0;
  } while( !isCorrectDataLine(pline,pargv,pargc) );

  toPointVec3[0] = getArgValueAsDouble(pline, 0);
  toPointVec3[1] = getArgValueAsDouble(pline, 1);
  toPointVec3[2] = getArgValueAsDouble(pline, 2);

  if( !rgbaIsInKeyWordLine )
  {
    if( getArgCount(pline) >= 7 )
    {
      toPointRGBAVec4[0] = getArgValueAsDouble(pline, 3);
      toPointRGBAVec4[1] = getArgValueAsDouble(pline, 4);   
      toPointRGBAVec4[2] = getArgValueAsDouble(pline, 5);
      toPointRGBAVec4[3] = getArgValueAsDouble(pline, 6);
    }
    else
    /* No color is indicated; make default opaque white */ 
    {
      toPointRGBAVec4[0] = 1.0;
      toPointRGBAVec4[1] = 1.0;
      toPointRGBAVec4[2] = 1.0;
      toPointRGBAVec4[3] = 1.0;
    }
  }

  while(1)
  {
    fromConnectivity = storePoint( fromPointVec3, fromPointRGBAVec4 );
    toConnectivity   = storePoint( toPointVec3, toPointRGBAVec4 );

    lineIsRedundant = storeConnectivity( fromConnectivity, toConnectivity );

    if( !lineIsRedundant )
    {
      if( rgbaIsInKeyWordLine )
      {
        printf("line %f %f %f %f\n", fromPointRGBAVec4[0],
	                             fromPointRGBAVec4[1],
	                             fromPointRGBAVec4[2],
	                             fromPointRGBAVec4[3]);

	printf("%f %f %f\n", fromPointVec3[0],
	                     fromPointVec3[1],
	                     fromPointVec3[2]);

	printf("%f %f %f\n", toPointVec3[0],
	                     toPointVec3[1],
	                     toPointVec3[2]);
      }
      else //RGBA is in every line, not on keyword line
      {
	printf("line\n");
    
	printf("%f %f %f %f %f %f %f\n", fromPointVec3[0],
	                                 fromPointVec3[1],
	                                 fromPointVec3[2],
	                                 fromPointRGBAVec4[0],
	                                 fromPointRGBAVec4[1],
	                                 fromPointRGBAVec4[2],
	                                 fromPointRGBAVec4[3]);

	printf("%f %f %f %f %f %f %f\n", toPointVec3[0],
	                                 toPointVec3[1],
	                                 toPointVec3[2],
	                                 toPointRGBAVec4[0],
	                                 toPointRGBAVec4[1],
	                                 toPointRGBAVec4[2],
	                                 toPointRGBAVec4[3]);	
      } //end else
    }

    if( atEOF() )
    {
      return 0;
    }
    else if( atEndOfShape )
    {
      return 1;
    }
    else
    {
      do
      {
	if(!getLine(pline))
	{
	  /*
	   * If the end of the file was reached while in a shape,
	   * (ie getLine returned 0) then assume it's the end of 
	   * the shape -- will be caught as such in if statement 
	   * below.
	   */
	  break;
	}
      } while( isBlankLine(pline) || containsComment(pline));

      fromPointVec3[0] = toPointVec3[0];
      fromPointVec3[1] = toPointVec3[1];
      fromPointVec3[2] = toPointVec3[2];

      if( !rgbaIsInKeyWordLine )
      {
	fromPointRGBAVec4[0] = toPointRGBAVec4[0];
	fromPointRGBAVec4[1] = toPointRGBAVec4[1];
	fromPointRGBAVec4[2] = toPointRGBAVec4[2];
	fromPointRGBAVec4[3] = toPointRGBAVec4[3];
      }

      if( (atEOF() && strlen(pline)<1) || !isCorrectDataLine(pline,pargv,pargc) )
      /*
       * End of file or end of shape found
       */
      {
	// tie together the last point and the first point
        toPointVec3[0] = firstPointVec3[0];
        toPointVec3[1] = firstPointVec3[1];
        toPointVec3[2] = firstPointVec3[2];

	if( !rgbaIsInKeyWordLine )
	{
	  toPointRGBAVec4[0] = firstPointRGBAVec4[0];
	  toPointRGBAVec4[1] = firstPointRGBAVec4[1];	
	  toPointRGBAVec4[2] = firstPointRGBAVec4[2];
	  toPointRGBAVec4[3] = firstPointRGBAVec4[3];
	}

	atEndOfShape = 1;
      }
      else
      {
	toPointVec3[0] = getArgValueAsDouble(pline, 0);
	toPointVec3[1] = getArgValueAsDouble(pline, 1);
	toPointVec3[2] = getArgValueAsDouble(pline, 2);

        if( !rgbaIsInKeyWordLine )
        {
          if( getArgCount(pline) >= 7 )
          {
            toPointRGBAVec4[0] = getArgValueAsDouble(pline, 3);
            toPointRGBAVec4[1] = getArgValueAsDouble(pline, 4);   
            toPointRGBAVec4[2] = getArgValueAsDouble(pline, 5);
            toPointRGBAVec4[3] = getArgValueAsDouble(pline, 6);
          }
          else
          /* No color is indicated; make default opaque white */ 
          {
            toPointRGBAVec4[0] = 1.0;
            toPointRGBAVec4[1] = 1.0;
            toPointRGBAVec4[2] = 1.0;
            toPointRGBAVec4[3] = 1.0;
          }
        }
      }
    } //end else
  } //end while
} //end parseShape

/* Prints information on usage*/
void usage()
{
    fprintf(stderr,
    "ERROR: Incorrect command line arguments\n"
    "\n"
    "USAGE:\n"
    "\tsavg-wireframe [no arguments]\n"
    "\tremoves surfaces so only lines remain (ie wireframe).\n");
}
