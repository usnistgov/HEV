/************************
* Savg-Octa		*	
* Created on 8/8/05	*
*			*
* This program creates a*
* regular octahedron that*
* is one unit tall.  It *
* takes no paramters and*
* will display an error *
* message if any are	*
* given.		*
************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "savgLineEvaluator.c"


#define MAX(x,y)  (((x)>(y))?(x):(y))


/* Prints information on usage*/


void usage()
	{
	fprintf (stderr, "\nUsage: savg-octa [-unit]\n\n");
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


	int len, i, useOrig;

	if (argc > 3)
		{
		fprintf (stderr, "savg-octa: Bad argument count.\n");
		usage ();
		exit (-1);
		}

	useOrig = 1;

	for (i = 1; i < argc; i++)
		{
		len = strlen (argv[i]);

		if (strncasecmp ("-unit", argv[i], MAX(2,len)) == 0)
			{
			useOrig = 0;
			}
		else
			{
			fprintf (stderr, 
			  "savg-octa: Unrecognized argument \"%s\".\n", 
			  argv[i]);
			usage ();
			exit (-1);
			}
		}



	double originalOctahedron[8][3][3] = 
	{

		{
			{-(M_SQRT2/4), -(M_SQRT2/4), 0},
			{(M_SQRT2/4), -(M_SQRT2/4), 0},
			{0, 0, .5}
		},

		{
			{(M_SQRT2/4), -(M_SQRT2/4), 0},
			{(M_SQRT2/4), (M_SQRT2/4), 0},
			{0, 0, .5}
		},


		{
			{(M_SQRT2/4), (M_SQRT2/4), 0},
			{-(M_SQRT2/4), (M_SQRT2/4), 0},
			{0, 0, .5}
		},

		{
			{-(M_SQRT2/4), (M_SQRT2/4), 0},
			{-(M_SQRT2/4), -(M_SQRT2/4), 0},
			{0, 0, .5}
		},

		{
			{(M_SQRT2/4), -(M_SQRT2/4), 0},
			{-(M_SQRT2/4), -(M_SQRT2/4), 0},
			{0, 0, -.5}
		},

		{
			{(M_SQRT2/4), (M_SQRT2/4), 0},
			{(M_SQRT2/4), -(M_SQRT2/4), 0},
			{0, 0, -.5}
		},

		{
			{-(M_SQRT2/4), (M_SQRT2/4), 0},
			{(M_SQRT2/4), (M_SQRT2/4), 0},
			{0, 0, -.5}
		},

		{
			{-(M_SQRT2/4), -(M_SQRT2/4), 0},
			{-(M_SQRT2/4), (M_SQRT2/4), 0},
			{0, 0, -.5}
		}

	};


	double unitOctahedron[8][3][3] = 
	{

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
		},

		{
			{0, 1, 0},
			{-1, 0, 0},
			{0, 0, 1}
		},

		{
			{-1, 0, 0},
			{0, -1, 0},
			{0, 0, 1}
		},

		{
			{0, -1, 0},
			{1, 0, 0},
			{0, 0, 1}
		},

		{
			{0, 1, 0},
			{1, 0, 0},
			{0, 0, -1}
		},

		{
			{-1, 0, 0},
			{0, 1, 0},
			{0, 0, -1}
		},

		{
			{0, -1, 0},
			{-1, 0, 0},
			{0, 0, -1}
		},

		{
			{1, 0, 0},
			{0, -1, 0},
			{0, 0, -1}
		}


	};


	for (i = 0; i < 8; i++)
		{
		writeTriangle (
			useOrig ? originalOctahedron[i] : unitOctahedron[i]);
		}






}

