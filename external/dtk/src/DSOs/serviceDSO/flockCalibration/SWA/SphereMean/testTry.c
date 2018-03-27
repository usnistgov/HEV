
#include <stdio.h>
#include "try.h"


main ()
	{
	double pt[2][4], t;
	double weight[2];
	double mean[4];

	while (1)
		{
		printf ("Enter A: ");
		scanf ("%lf %lf %lf %lf",
			&(pt[0][0]),
			&(pt[0][1]),
			&(pt[0][2]),
			&(pt[0][3]) );

		printf ("Enter B: ");
		scanf ("%lf %lf %lf %lf",
			&(pt[1][0]),
			&(pt[1][1]),
			&(pt[1][2]),
			&(pt[1][3]) );

		printf ("Enter t: ");
		scanf ("%lf", &t);

		weight[0] = 1-t;
		weight[1] = t;

		sphericalMean4 (2, pt, weight, mean);
		printf ("mean = %lf %lf %lf %lf\n",
			mean[0],
			mean[1],
			mean[2],
			mean[3]);

		printf ("\n\n\n");


		}  // end of while 1

	}  // end of main 







