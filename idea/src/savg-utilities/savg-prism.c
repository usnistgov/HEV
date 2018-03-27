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
 * savg-prism				        *
 * Modified version of savg-cone by		*
 * Adele Peskin   	                	*
 * Created on: 7-20-05  			*
 * Current Version: 1.00			*
 *						*
 * Description:	                                *
 *    savg-prism creates a unit-sized           *
 * (radius of one unit) base in savg format.    *
 * The prism is centered at the                 *
 * origin. It takes up to four command line     *
 * arguments, -n val, height, width of top      *
 * base in comparision to bottom, and open/     *
 * closed faces.                         	* 
 * -n val indicades the number of sides, height	*
 * corresponds to height (default = 1), and	*
 * the topwidth is a percentage of the		*
 * bottom base's size. The parameter "top"      *
 * creates a top face, "bottom" creates a       *
 * bottom face, open creates none,              *
 * and default creates both faces.              *
 * To manipulate the presence of faces, -n val, *
 * height, and top width MUST BE SPECIFIED.     *
 * In default mode,		                *
 * this program functions much like 		*
 * savg-cylinder, but the user can change	*
 * the width of the top base to make		*
 * different shapes than allowed by the 	*
 * cylinder program.				* 
 ************************************************/

#if 0


Creates a polyhedron made of a regular n-sided polygonal base, 
a top polygon which is a translated and scaled copy of that base, 
and n quadrilateral faces joining corresponding sides of the base and
top. 
The base polygon is centered at the origin and lies on the XY plane.
The top polygon is a copy of the base polygon translated along
the positive Z axis, and scaled about its center.  The amount of
translation and scaling is controlled by command line arguments.

Usage:

   savg-prism [nSides] [height] [scale] [face]

   nSides  -  number of sides of the base and top polygon (default is 16)
   height  -  the amount of translation of the top polygon (default is 1)
   scale   -  the amount that the top polygon is scaled (default is 1)
   face    -  indicates whether to include the base and top
               polygons
		top       - include top but not bottom
		bottom    - include bottom but not top
		open      - include neither top nor bottom
		(default) - include both top and bottom


#endif



#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "savgLineEvaluator.c"

#define PI 3.141592653589793238462643


static void
usage ()
	{
	fprintf (stderr, 
		"Usage: savg-prism [nSides] [height] [scale] [face]\n");
	return;
	}  // end of usage


int main(argc, argv)
int argc;
char *argv[];
{
  int i;
  int npoints = 16;    /* no. of polygons requested */
  int face=3;
  float height = 1;
  float topw = 1; /* top, percentage width of base */
  double newX, newY, newX2, newY2, angle, angle2;
  double u[3], v[3], norm[3], dummy, dot;
 
/*
 * COMMAND LINE PROCESSING
 */

#if 0


Creates a polyhedron made of a regular n-sided polygonal base, 
a top polygon which is a translated and scaled copy of that base, 
and n quadrilateral faces joining corresponding sides of the base and
top. 
The base polygon is centered at the origin and lies on the XY plane.
The top polygon is a copy of the base polygon translated along
the positive Z axis, and scaled about its center.  The amount of
translation and scaling is controlled by command line arguments.

Usage:

   savg-prism [nSides] [height] [scale] [face]

   nSides  -  number of sides of the base and top polygon (default is 16)
   height  -  the amount of translation of the top polygon (default is 1)
   scale   -  the amount that the top polygon is scaled (default is 1)
   face    -  indicates whether to include the base and top
               polygons
		top       - include top but not bottom
		bottom    - include bottom but not top
		open      - include neither top nor bottom
		(default) - include both top and bottom


#endif



  if (argc > 1) {
    // first parameter is number of polygons
    npoints = atoi(argv[1]);
   }

  if (argc == 3 )
     { 
         //next parameter is height
        npoints = atoi(argv[1]); 
	height = atof(argv[2]);
    }


  if (argc == 4)
  {
	//next parameter is width of top base
        npoints = atoi(argv[1]);
        height = atof(argv[2]);
        topw = atof(argv[3]);
  }

  if(argc ==5){  
    if (strcmp(argv[4], "top")==0){
     face=1;
      npoints = atoi(argv[1]);
        height = atof(argv[2]);
        topw = atof(argv[3]);
   }
   else if(strcmp(argv[4], "bottom")==0){
    face=2;
      npoints = atoi(argv[1]);
        height = atof(argv[2]);
        topw = atof(argv[3]);
   }
    else if(strcmp(argv[4], "open")==0){
      face=0;
      npoints = atoi(argv[1]);
        height = atof(argv[2]);
        topw = atof(argv[3]);
    }
    else{
      fprintf (stderr, 
	"savg-prism: Error in command line argument \"%s\".\n", argv[4]);
      usage();
      exit(EXIT_FAILURE);
    }
  }

  if (argc > 5){
      fprintf (stderr, "savg-prism: Bad argument count.\n");
    usage();
    exit(EXIT_FAILURE);
  }
  
/*
 * BODY
 */

  // generate the base polygon
  if( face ==2 || face ==3) {
    printf("polygons\n");
    for (i = 0; i < npoints; ++i) {
      // find the angle in radians
      angle = - (double)(i*360/npoints) * PI/180;
      newX = (.5)*(cos(angle));
      newY = (.5)*(sin(angle));
      printf("%f %f 0.0 0.0 0.0 -1.0\n",newX,newY);
    }
  }
  
  // now create the (number of npoints) rectangles and their normals
  for (i = 0; i < npoints; ++i) {
    printf("polygons\n");
    // find the angle in radians
    angle = (double)(i*360/npoints) * PI/180;
    angle2 = (double)((i+1)*360/npoints) * PI/180;
    newX = (.5)*(cos(angle));
    newY = (.5)*(sin(angle));
    newX2 = (.5)*(cos(angle2));
    newY2 = (.5)*(sin(angle2));

    u[0] = newX - newX2;
    u[1] = newY - newY2;
    u[2] = 0;

    v[0] = newX2 - (topw * newX2);
    v[1] = newY2 - (topw * newY2);
    v[2] = 0 - height;

    calculateCrossProduct (u, v, norm, dummy);

    dot = norm[0]*newX + norm[1]*newY;
    if (dot < 0)
	{
	norm[0] *= -1;
	norm[1] *= -1;
	norm[2] *= -1;
	}


//enter node i,i+height
    printf("%f %f 0.0  %g %g %g\n",newX,newY,
		norm[0], norm[1], norm[2] );
    printf("%f %f 0.0  %g %g %g\n",newX2,newY2,
		norm[0], norm[1], norm[2] );
    printf("%f %f %f  %g %g %g\n",(topw * newX2), (topw * newY2),height,
		norm[0], norm[1], norm[2] ); 
    printf("%f %f %f  %g %g %g\n",(topw * newX), (topw * newY),height,
		norm[0], norm[1], norm[2] ); 
  
}


  // now create the top face
       if(face ==1 || face==3){
  printf("polygons\n");
  for (i = 0; i< npoints; ++i)
  {
      angle = (double)(i*360/npoints) * PI/180;
      newX = (.5)*(cos(angle));
      newY = (.5)*(sin(angle));
      printf("%f %f %f 0 0 1\n", (topw * newX), (topw * newY), height);
  }
       }
}



