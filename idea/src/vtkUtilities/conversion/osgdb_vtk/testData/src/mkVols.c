
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


static void
addNoise (double noiseSD, double *val)
        {

        // Wrapper for RGAUSS, a fortran routine from CMLIB for generating
        // a normally distributed sequence of random numbers.

        void rgauss_wrapper_ (double *xmean, double *sd, double *outVal);
        double outVal;
        double sd;
        double xMean = 0.0;

        sd = noiseSD;

        rgauss_wrapper_ (&xMean, &sd, &outVal);

        *val += outVal;

        } // end of addNoise


int
main ()
        {
        // int gridDim[3] = {8, 4, 15};
        int gridDim[3] = {16, 10, 33};

        for (int k = 0; k < gridDim[2]; k++)
            {
            for (int j = 0; j < gridDim[1]; j++)
                {
                for (int i = 0; i < gridDim[0]; i++)
                        {

                        double x = 1.05 * (((double)i) / (gridDim[0]-1.0));
                        double y = 1.05 * (((double)j) / (gridDim[1]-1.0));
                        double z = 1.05 * (((double)k) / (gridDim[2]-1.0));

                        double val = sqrt (x*x + y*y + z*z);
                        addNoise (0.001, &val);
                        printf ("%.18e\n", val);
                        }  // end of k loop
                }  // end of j loop
            }  // end of i loop


        } // end of main

