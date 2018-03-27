// This example uses class dtkDistribution

#include <dtk.h>

#define NUM_BINS    400

int main(int argc, char **argv)
{
  dtkDistribution dist(NUM_BINS, -1.1, 1.1, 1.0);
  double x;

  for(x=0;x<2*M_PI;x+=0.0001)
    dist.put(sin(x));

  fprintf(stderr,"Pipe stdout to a plotter:\n");

  dist.calculate();
  for(int i=0;i<NUM_BINS;i++)
    printf("%.16g %.16g %.16g %.16g\n",
	   dist.x[i], dist.ynorm[i], dist.y[i], dist.sum[i]);

  fprintf(stderr,"Pipe stdout to a plotter:\n");
  fprintf(stderr,"There were %d values put in the distribution.\n",
	  dist.numberOfValues());

  return 0;
}
