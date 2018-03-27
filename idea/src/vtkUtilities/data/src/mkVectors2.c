
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


static void
normalize (double v[3])
    {
    double len = sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
    }

int
main ()
        {
        int dim[3];
        double center[3];
        double v[3];
        int i, j, k;

        dim[0] = 10;
        dim[1] = 12;
        dim[2] = 11;
        center[0] = (dim[0] - 1.0)/2.0;
        center[1] = (dim[1] - 1.0)/2.0;
        center[2] = (dim[2] - 1.0)/2.0;

        for (k = 0; k < dim[2]; k++)
            {
            for (j = 0; j < dim[1]; j++)
                {
                for (i = 0; i < dim[0]; i++)
                    {
                    v[0] = - (j - center[1]);
                    v[1] =   (i - center[0]);
                    v[2] = k + 1;
                    v[2] *= dim[2]/100.0;
                    normalize (v);
                    v[0] *= (k+1.0)/(dim[2]+1.0);
                    v[1] *= (k+1.0)/(dim[2]+1.0);
                    v[2] *= (k+1.0)/(dim[2]+1.0);
                    printf ("%.9g %.9g %.9g\n", v[0], v[1], v[2]);
                    
                    }
                }
            }

        }

