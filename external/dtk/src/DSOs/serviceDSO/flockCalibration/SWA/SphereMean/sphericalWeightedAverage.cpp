
#include <stdio.h>

#include "SphereMean.h"
#include "sphericalWeightedAverage.h"


#define MAX_PTS   (1000)



extern "C"
{

int
sphericalWeightedAverage4 
  (long numPts, double pt[][4], double weight[], double mean[4])
	{
	static VectorR4 v[MAX_PTS];
	long i;

	if (numPts > MAX_PTS)
		{
		mean[0] = mean[1] = mean[2] = mean[3] = 0.0;
		return -1;
		}

	for (i = 0; i < numPts; i++)
		{
		v[i].Load (pt[i]);
		}

	ComputeMeanSphere (numPts, v, weight) . Dump (mean);

	return 0;
	}	// end of sphericalMean4


}  // end of extern "C"



