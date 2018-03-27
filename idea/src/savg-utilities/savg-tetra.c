/****************************************
* Savg-tetra				*
* Created on 8/5/05			*
*					*
* This program creates a glyph 		*
* model of a unit sized tetrahedron	*
* which takes no arguments. If 		*
* arguments are present, an error	*
* message will appear.			*
****************************************/ 


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "savgLineEvaluator.c"

#define SQRT3 (1.7320508075688)

#define MAX(x,y)  (((x)>(y))?(x):(y))

/* Prints information on usage*/

void usage()
{
	fprintf(stderr, "Usage: savg-tetra [-unit] \n\n");
}



static void normalize (double v[3])
	{
	double len = sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if (len != 0)
		{
		v[0] /= len;
		v[1] /= len;
		v[2] /= len;
		}
	}  // end of normalize




static void
writeTriangle (double tri[3][3])
        {
        double norm[3];
        double dummy, dot;
        int i;

        double u[3], v[3];

        for (i = 0; i < 3; i++)
                {
                u[i] = tri[1][i] - tri[0][i];
                v[i] = tri[2][i] - tri[1][i];
                }

        calculateCrossProduct (u, v, norm, dummy);
        dot = tri[0][0]*norm[0] + tri[0][1]*norm[1] + tri[0][2]*norm[2];
        if (dot < 0)
                {
                norm[0] *= -1;
                norm[1] *= -1;
                norm[2] *= -1;
                }

        normalize (norm);

        printf ("polygon\n");
        for (i = 0; i < 3; i++)
                {
                printf ("%g %g %g  %g %g %g\n",
                        tri[i][0], tri[i][1], tri[i][2],
                        norm[0], norm[1], norm[2]
                        );
                }

        }  // end of writeTriangle






int main(argc, argv)
int argc;
char *argv[];
{

	int i, j, k;
	double tri[3][3];
	int useOrig;

	if(argc > 2){
		fprintf (stderr, "savg-tetra: Bad argument count.\n");
		usage();
		exit(EXIT_FAILURE);
	}


	useOrig = 1;

	for (i = 1; i < argc; i++)
		{
		if (strncasecmp (argv[i], "-unit", MAX (strlen(argv[i]),2)) == 0)
			{
			useOrig = 0;
			}
		else
			{
			fprintf (stderr, 
			  "savg-tetra: Unrecognized argument \"%s\".\n", 
			  argv[i]);
			usage ();
			exit (EXIT_FAILURE);
			}
		}



#if 0
	double tetrahedron[4][3][3] =
	{
	   {

		{ (SQRT3*M_SQRT2/4),  -(M_SQRT2/4),  0},
		{-(SQRT3*M_SQRT2/4),  -(M_SQRT2/4),  0},
		{0,                    (M_SQRT2/2),  0}
	   },


	   {
		{ (SQRT3*M_SQRT2/4),   -(M_SQRT2/4), 0},
		{0,                     (M_SQRT2/2), 0},
		{0,                     0,           1}
	   },


	   {
		{0,                     (M_SQRT2/2), 0},
		{-(SQRT3*M_SQRT2/4),   -(M_SQRT2/4), 0},
		{0,                     0,           1}
	   },


	   {
		{-(SQRT3*M_SQRT2/4),   -(M_SQRT2/4), 0},
		{ (SQRT3*M_SQRT2/4),   -(M_SQRT2/4), 0},
		{0,                     0,           1}
	   }
	};
#endif


	double origTetraVertices[4][3] =
		{
			{ 0.5,  -SQRT3/6,            0.0},
			{-0.5,  -SQRT3/6,            0.0},
			{ 0.0,   SQRT3/3,            0.0},
			{ 0.0,       0.0,  M_SQRT2/SQRT3},
		};

#if 0
	double unitTetraVertices[4][3] =
		{
			{  SQRT3/2, -0.5,  -M_SQRT2/4},
			{ -SQRT3/2, -0.5,  -M_SQRT2/4},
			{        0,  1.0,  -M_SQRT2/4},
			{        0,  0.0, 3*M_SQRT2/4}
		};
#else

	double unitTetraVertices[4][3] =
		{
			{  M_SQRT2*SQRT3/3,   -M_SQRT2/3,  -(1.0/3.0) },
			{ -M_SQRT2*SQRT3/3,   -M_SQRT2/3,  -(1.0/3.0) },
			{                0,  2*M_SQRT2/3,  -(1.0/3.0) },
			{                0,            0,           1 }
		};



#endif








	int tvi[4][3] = 
		{
			{0, 1, 2},
			{0, 2, 3},
			{2, 1, 3},
			{1, 0, 3}
		};

	for (i = 0; i < 4; i++)
		{
		for (j = 0; j < 3; j++)
			{
			for (k = 0; k < 3; k++)
				{
				
				tri[j][k] = useOrig ?
					origTetraVertices [ tvi[i][j] ] [k] :
					unitTetraVertices [ tvi[i][j] ] [k]   ;
				}
			}

		writeTriangle (tri);

		}

	
}   // end of main 

