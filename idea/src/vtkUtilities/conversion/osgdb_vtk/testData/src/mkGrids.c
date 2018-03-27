
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


static void
writeSPHeader  (FILE *fp, int dim[3], char *desc)
        {

        fprintf (fp, "# vtk DataFile Version 2.0\n");
        fprintf (fp, "%s\n", desc);
        fprintf (fp, "ASCII\n");
        fprintf (fp, "\n");

        // Structure :  Geometry / Topology
        fprintf (fp, "DATASET STRUCTURED_POINTS\n");
        fprintf (fp, "DIMENSIONS %d %d %d\n", dim[0], dim[1], dim[2]  );
        fprintf (fp, "ORIGIN 1 1 1\n");
        fprintf (fp, "SPACING 1 1 1\n");

        // Dataset Attributes
        fprintf (fp, "POINT_DATA %d\n", dim[0] * dim[1] * dim[2] );
        fprintf (fp, "SCALARS scalars double\n");
        fprintf (fp, "LOOKUP_TABLE default\n");

        }  // end of writeSPHeader



static int
writeRCoords (FILE *fp, char dir, int dim, int reverse)
        {
        int i;
        fprintf (fp, "%c_COORDINATES %d float\n", dir, dim);

        for (i = 0; i < dim; i++)
                {
                float fi = reverse ? i : ((dim-1) - i);
                fprintf (fp, "%g\n", (fi*fi)/(dim-1));
                }

        }  // end of writeRCoords

static void
writeRGHeader  (FILE *fp, int dim[3], char *desc)
        {

        fprintf (fp, "# vtk DataFile Version 2.0\n");
        fprintf (fp, "%s\n", desc);
        fprintf (fp, "ASCII\n");
        fprintf (fp, "\n");

        // Structure :  Geometry / Topology
        fprintf (fp, "DATASET RECTILINEAR_GRID\n");
        fprintf (fp, "DIMENSIONS %d %d %d\n", dim[0], dim[1], dim[2]  );
        writeRCoords (fp, 'X', dim[0], 1);
        writeRCoords (fp, 'Y', dim[1], 0);
        writeRCoords (fp, 'Z', dim[2], 1);


        // Dataset Attributes
        fprintf (fp, "POINT_DATA %d\n", dim[0] * dim[1] * dim[2] );
        fprintf (fp, "SCALARS scalars double\n");
        fprintf (fp, "LOOKUP_TABLE default\n");

        }  // end of writeRGHeader


static void
writeSGHeader  (FILE *fp, int dim[3], char *desc, int noise)
        {
        int i, j, k;

        fprintf (fp, "# vtk DataFile Version 2.0\n");
        fprintf (fp, "%s\n", desc);
        fprintf (fp, "ASCII\n");
        fprintf (fp, "\n");

        // Structure :  Geometry / Topology
        fprintf (fp, "DATASET STRUCTURED_GRID\n");
        fprintf (fp, "DIMENSIONS %d %d %d\n", dim[0], dim[1], dim[2]  );
        fprintf (fp, "POINTS %d float\n", dim[0] * dim[1] * dim[2]  );
        for (k = 0; k < dim[2]; k++)
            {
            for (j = 0; j < dim[1]; j++)
                {
                for (i = 0; i < dim[0]; i++)
                        {
                        double x = i;
                        double y = j;
                        double z = k;
                        if (noise)
                                {
                                x += (drand48()-0.5)/4;
                                y += (drand48()-0.5)/4;
                                z += (drand48()-0.5)/4;
                                }
                        y += (dim[1] - 1) * sin ( (z/(dim[2]-1))*2*M_PI );
                        fprintf (fp, "%g %g %g\n", x, y, z);
                        }
                }
            }
        


        // Dataset Attributes
        fprintf (fp, "POINT_DATA %d\n", dim[0] * dim[1] * dim[2] );
        fprintf (fp, "SCALARS scalars double\n");
        fprintf (fp, "LOOKUP_TABLE default\n");

        }  // end of writeSGHeader




static void
writeGridData (FILE *fp, int dim[3], double center[3], int noise)
        {
        int i, j, k;

        for (k = 0; k < dim[2]; k++)
            {
            for (j = 0; j < dim[1]; j++)
                {
                for (i = 0; i < dim[0]; i++)
                        {

                        double x = ((i-center[0]) / (dim[0]-1.0));
                        double y = ((j-center[1]) / (dim[1]-1.0));
                        double z = ((k-center[2]) / (dim[2]-1.0));
                        double val = 0.8 * sqrt (x*x + y*y + z*z);
                        if (noise)
                                {
                                addNoise (0.005, &val);
                                }
                        fprintf (fp, "%.18g\n", val);
                        }  // end of k loop
                }  // end of j loop
            }  // end of i loop


        } // end of writeGridData




static int
writeSP (char *fn, int dim[3], double center[3], int noise)
        {
        FILE *fp;
        char desc[1000];

        fp = fopen (fn, "w");
        if (fp == NULL)
                {
                fprintf (stderr, "Unable to open file %s.\n", fn);
                return -1;
                }

        sprintf (desc, "Ellipsoid: dims %d %d %d  center %g %g %g  noise %s",
                dim[0], dim[1], dim[2], center[0], center[1], center[2],
                noise?"yes":"no");
        writeSPHeader (fp, dim, desc);
        writeGridData (fp, dim, center, noise);

        fclose (fp);
        return 0;
        }       // end of writeSP



static int
writeRG (char *fn, int dim[3], double center[3], int noise)
        {
        FILE *fp;
        char desc[1000];

        fp = fopen (fn, "w");
        if (fp == NULL)
                {
                fprintf (stderr, "Unable to open file %s.\n", fn);
                return -1;
                }

        sprintf (desc, 
          "RG Distorted Ellipsoid: dims %d %d %d  center %g %g %g  noise %s",
                dim[0], dim[1], dim[2], center[0], center[1], center[2],
                noise?"yes":"no");
        writeRGHeader (fp, dim, desc);
        writeGridData (fp, dim, center, noise);

        fclose (fp);
        return 0;
        }       // end of writeRG


static int
writeSG (char *fn, int dim[3], double center[3], int noise)
        {
        FILE *fp;
        char desc[1000];

        fp = fopen (fn, "w");
        if (fp == NULL)
                {
                fprintf (stderr, "Unable to open file %s.\n", fn);
                return -1;
                }

        sprintf (desc, 
          "SG Distorted Ellipsoid: dims %d %d %d  center %g %g %g  noise %s",
                dim[0], dim[1], dim[2], center[0], center[1], center[2],
                noise?"yes":"no");
        writeSGHeader (fp, dim, desc, noise);
        writeGridData (fp, dim, center, noise);

        fclose (fp);
        return 0;
        }       // end of writeRG


// Unstructured grids are quite different than the other grids, so
// I'll generate the sample grid in a very different way.  
// I'm going to make a cylinder along the z axis with a specified
// number of z levels.  The data values assigned to the points will
// change from level to level to give a helical arrangement.

writeUGHeader (
  FILE *fp, 
  int radDiv, 
  int levels, 
  char *desc, 
  int noise
  )
        {
        int i, j;

        fprintf (fp, "# vtk DataFile Version 2.0\n");
        fprintf (fp, "%s\n", desc);
        fprintf (fp, "ASCII\n");
        fprintf (fp, "\n");

        // Structure :  Geometry / Topology
        fprintf (fp, "DATASET UNSTRUCTURED_GRID\n");
        fprintf (fp, "POINTS %d float\n", radDiv*levels);

        // calc nominal length of edge
        double xx = 1.0 - cos ((1.0/radDiv) * 2 * M_PI);
        double yy = sin ((1.0/radDiv) * 2 * M_PI);
        double len = sqrt (yy*yy + xx*xx);


        double radius = (levels-1)/6.0;
        for (j = 0; j < levels; j++)
                {
                for (i = 0; i < radDiv; i++)
                        {
                        double z = j;
                        double ang = (((double)i)/radDiv) * 2 * M_PI ;
                        double x = radius * cos (ang);
                        double y = radius * sin (ang);
                        if (noise)
                                {
                                x += len*(drand48()-0.5)/2;
                                y += len*(drand48()-0.5)/2;
                                z += (drand48()-0.5)/2;
                                }

                        fprintf (fp, "%g %g %g\n", x, y, z);
                        }
                }

        int numTri = radDiv*2*(levels-1);
        fprintf (fp, "CELLS %d %d\n", numTri, 4*numTri);
        for (j = 0; j < (levels-1); j++)
                {
                int lowerStart = radDiv*j;
                int upperStart = lowerStart + radDiv;
                for (i = 0; i < radDiv; i++)
                        {
                        int lower = radDiv*j + i;
                        int upper = lower + radDiv;
                        fprintf (fp, "3 %d %d %d \n",
                                i + lowerStart,
                                (i+1)%radDiv + lowerStart,
                                (i+1)%radDiv + upperStart);
                        fprintf (fp, "3 %d %d %d \n",
                                (i+1)%radDiv + upperStart,
                                i + upperStart,
                                i + lowerStart);
                                
                        }
                }

        fprintf (fp, "CELL_TYPES %d\n", numTri);
        for (j = 0; j < (levels-1); j++)
                {
                for (i = 0; i < radDiv; i++)
                        {
                        fprintf (fp, "5\n");
                        fprintf (fp, "5\n");
                        }
                }




        } // end of writeUGHeader


static void
writeUGPtData (FILE *fp, int radDiv, int levels, int noise)
        {
        int i, j;
        
        fprintf (fp, "POINT_DATA %d\n", radDiv*levels);
        fprintf (fp, "SCALARS scalars double\n");
        fprintf (fp, "LOOKUP_TABLE default\n");

        for (j = 0; j < levels; j++)
                {
                for (i = 0; i < radDiv; i++)
                        {
                        double a = (i == 0) ? radDiv : i;
                        a -= 0.63*j;
                        while (a < 0)
                                {
                                a += radDiv;
                                }
                        double val;
                        if (a <= (radDiv/2.0))
                                {
                                val = a/(radDiv/2.0);
                                }
                        else
                                {
                                val = (radDiv - a) / (radDiv/2.0);
                                }

                        if (noise)
                                {
                                addNoise (0.005, &val);
                                }
                        fprintf (fp, "%g\n", val);
                        }
                }

        }  // end of writeUGData

writeUG (char *fn, int numRadialDivisions, int numLevels, int noise)
        {
        FILE *fp;
        char desc[1000];

        fp = fopen (fn, "w");
        if (fp == NULL)
                {
                fprintf (stderr, "Unable to open file %s.\n", fn);
                return -1;
                }

        sprintf (desc, 
          "UG Cylinder: radial divisions %d   levels %d   noise %s",
                numRadialDivisions, numLevels, noise?"yes":"no");

        writeUGHeader (fp, numRadialDivisions, numLevels, desc, noise);
        writeUGPtData (fp, numRadialDivisions, numLevels, noise);

        fclose (fp);
        return 0;

        } // end of writeUG





static void
centerInDim (int dim[3], double center[3])
        {
        center[0] = (dim[0]-1)/2.0;
        center[1] = (dim[1]-1)/2.0;
        center[2] = (dim[2]-1)/2.0;
        }
int
main ()
        {
        int dim[3];
        double center[3];

        dim[0] = 16;
        dim[1] = 10;
        dim[2] = 33;
        center[0] = center[1] = center[2] = 0;
        writeSP ("structPoints.1.vtk", dim, center, 0);
        writeSP ("structPoints.1.noise.vtk", dim, center, 1);
        writeRG ("rectGrid.1.vtk", dim, center, 0);
        writeRG ("rectGrid.1.noise.vtk", dim, center, 1);
        writeSG ("structGrid.1.vtk", dim, center, 0);
        writeSG ("structGrid.1.noise.vtk", dim, center, 1);

        centerInDim (dim, center);
        writeSP ("structPoints.2.vtk", dim, center, 0);
        writeSP ("structPoints.2.noise.vtk", dim, center, 1);
        writeRG ("rectGrid.2.vtk", dim, center, 0);
        writeRG ("rectGrid.2.noise.vtk", dim, center, 1);
        writeSG ("structGrid.2.vtk", dim, center, 0);
        writeSG ("structGrid.2.noise.vtk", dim, center, 1);


        writeUG ("unstructGrid.1.vtk", 30, 30, 0);
        writeUG ("unstructGrid.1.noise.vtk", 30, 30, 1);

        }  // end of main

