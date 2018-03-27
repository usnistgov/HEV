
/*
!
!                     Copyright Notice
!
! This software was developed at the National Institute of Standards
! and Technology (NIST) by employees of the Federal Government in the
! course of their official duties. Pursuant to title 17 Section 105
! of the United States Code this software is not subject to copyright
! protection and is in the public domain.
!
! This is an experimental system.  NIST assumes no responsibility
! whatsoever for its use by other parties, and makes no guarantees,
! expressed or implied, about its quality, reliability, or any other
! characteristic.
!
! We would appreciate acknowledgment if the software is used.
!
*/

/*
This program reads in the specification for a gridded data set
based on a quadric surface.  It writes out a vtk file containing
the gridded data.


*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "quadric.h"
#include "getQuadParams.h"



///////////////////////////////////////////////////////////////////



static void
writeVtkHeader  (FILE *fp, QuadricGrid *quadricGrid)
        {

        fprintf (fp, "# vtk DataFile Version 2.0\n");
        fprintf (fp, "%s\n", quadricGrid->oneLineDesc);
        fprintf (fp, "ASCII\n");
        fprintf (fp, "\n");

        // Structure :  Geometry / Topology
        fprintf (fp, "DATASET STRUCTURED_POINTS\n");
        fprintf (fp, "DIMENSIONS %d %d %d\n",
                        quadricGrid->gridDim[0],
                        quadricGrid->gridDim[1],
                        quadricGrid->gridDim[2]  );
        fprintf (fp, "ORIGIN %f %f %f\n",
                        quadricGrid->gridToCoordOffset[0],
                        quadricGrid->gridToCoordOffset[1],
                        quadricGrid->gridToCoordOffset[2]  );
        fprintf (fp, "SPACING %f %f %f\n",
                        quadricGrid->gridToCoordScale[0],
                        quadricGrid->gridToCoordScale[1],
                        quadricGrid->gridToCoordScale[2]  );

        // Dataset Attributes
        fprintf (fp, "POINT_DATA %d\n",
                        quadricGrid->gridDim[0]*
                        quadricGrid->gridDim[1]*
                        quadricGrid->gridDim[2]  );
        fprintf (fp, "SCALARS scalars double\n");
        fprintf (fp, "LOOKUP_TABLE default\n");


        }  // end of writeVtkHeader

static void
writeVtkPoint (FILE *fp, double val)
        {
        fprintf (fp, "%.18e\n", val);
        }




static void
getSubsampLocs ( GridValueSpec *gValSpec, double voxelScale[3])
        {
        double x, y, z;
        int i;
        double drand48(void);

        gValSpec->subsampLocsRel = 
                 (Pt3D *) malloc (gValSpec->voxelSamplingCount * sizeof (Pt3D));
        if (gValSpec->subsampLocsRel == NULL)
                {
                fprintf (stderr, 
                           "Unable to allocate subsample grid.\n");
                exit (-1);
                }

        if (gValSpec->footprint == FP_BOX)
                {
                for (i = 0; i < gValSpec->voxelSamplingCount; i++)
                    {
                    x = (drand48()-0.5) * 
                                voxelScale[0] * gValSpec->footprintScale;
                    y = (drand48()-0.5) * 
                                voxelScale[1] * gValSpec->footprintScale;
                    z = (drand48()-0.5) * 
                                voxelScale[2] * gValSpec->footprintScale;
                    gValSpec->subsampLocsRel[i][0] = x;
                    gValSpec->subsampLocsRel[i][1] = y;
                    gValSpec->subsampLocsRel[i][2] = z;
                    }



                }
        else
                {
                for (i = 0; i < gValSpec->voxelSamplingCount; )
                    {
                    x = (drand48()-0.5);
                    y = (drand48()-0.5);
                    z = (drand48()-0.5);
                    if ( (x*x + y*y + z*z) <= 0.25 )
                        {
                        gValSpec->subsampLocsRel[i][0] = 
                                x * voxelScale[0] * gValSpec->footprintScale;
                        gValSpec->subsampLocsRel[i][1] = 
                                y * voxelScale[1] * gValSpec->footprintScale;
                        gValSpec->subsampLocsRel[i][2] = 
                                z * voxelScale[2] * gValSpec->footprintScale;
                        i++;
                        }
                    }
                }



        }  // end of getSubSampLocs


static int
isInside (QuadricGrid *qGrid, double xyz[3])
        {
        // isInside actually means that the quadric equation
        // evaluates to <= 0.

        double x = xyz[0];
        double y = xyz[1];
        double z = xyz[2];

        double p =
                        qGrid->qShape.qc[0] * x*x  +
                        qGrid->qShape.qc[1] * y*y  +
                        qGrid->qShape.qc[2] * z*z  +
                        qGrid->qShape.qc[3] * y*z  +
                        qGrid->qShape.qc[4] * x*z  +
                        qGrid->qShape.qc[5] * x*y  +
                        qGrid->qShape.qc[6] * x    +
                        qGrid->qShape.qc[7] * y    +
                        qGrid->qShape.qc[8] * z    +
                        qGrid->qShape.qc[9];

        return (p <= 0.0);

        }  // end of isInside


static double
subsample (QuadricGrid *quadricGrid, double xyz[3])
        {


        int inCount = 0;
        double inP;
        double xyzLoc[3];
        Pt3D *relLoc;
        int i;

        relLoc = quadricGrid->gValSpec.subsampLocsRel;

        for (i = 0; i < quadricGrid->gValSpec.voxelSamplingCount; i++)
                {
                xyzLoc[0] = xyz[0] + relLoc[i][0];
                xyzLoc[1] = xyz[1] + relLoc[i][1];
                xyzLoc[2] = xyz[2] + relLoc[i][2];
                if (isInside (quadricGrid, xyzLoc))
                        {
                        inCount++;
                        }
                }

        inP = ((double)inCount) / 
                  ((double)quadricGrid->gValSpec.voxelSamplingCount);


        return  (inP * quadricGrid->gValSpec.insideVal) +  
                          ((1-inP) * quadricGrid->gValSpec.outsideVal) ;

        }  // end of subsample



static void
ijkToXyz (QuadricGrid *quadricGrid, int ijk[3], double xyz[3])
        {

        xyz[0] = ijk[0]*quadricGrid->gridToCoordScale[0] + 
                                quadricGrid->gridToCoordOffset[0];

        xyz[1] = ijk[1]*quadricGrid->gridToCoordScale[1] + 
                                quadricGrid->gridToCoordOffset[1];

        xyz[2] = ijk[2]*quadricGrid->gridToCoordScale[2] + 
                                quadricGrid->gridToCoordOffset[2];

        } // end of ijkToXyz





static void
addNoise (double noiseSD, double *val)
        {

        // Wrapper for RGAUSS, a fortran routine from CMLIB for generating
        // a normally distributed sequence of random numbers.

        void rgauss_wrapper_ (double *xmean, double *sd, double *outVal);

        // Could probably find a better random number generator, but this
        // is OK for our purposes.
        // Alternatives: RNOR; UNI+Box/Muller; drand48+Box/Muller


        double outVal;
        double sd;
        double xMean = 0.0;

        sd = noiseSD;

        rgauss_wrapper_ (&xMean, &sd, &outVal);

        *val += outVal;

        } // end of addNoise


static char *
qShapeTypeToStr (enum quadricType qType)
        {

        switch (qType)
                {
                case ELLIPSOID:
                        return "ELLIPSOID";

                case ELLIPTIC_PARABOLOID:
                        return "ELLIPTIC_PARABOLOID";

                case HYPERBOLIC_PARABOLOID:
                        return "HYPERBOLIC_PARABOLOID";

                case HYPERBOLOID_ONE_SHEET:
                        return "HYPERBOLOID_OF_ONE_SHEET";

                case HYPERBOLOID_TWO_SHEETS:
                        return "HYPERBOLOID_OF_TWO_SHEETS";

                default:
                        // should never get here
                        fprintf (stderr,
                          "Bad surface type in qShapeTypeToStr.\n");
                        exit (-1);
                }


        }  // end of qShapeTypeToStr



static void
deriveQuadParams (QuadricGrid *qGrid)
        {
        sprintf (qGrid->oneLineDesc, "%s %f %f %f\n", 
                        qShapeTypeToStr (qGrid->qShape.qType),
                        qGrid->qShape.ABC[0],
                        qGrid->qShape.ABC[1],
                        qGrid->qShape.ABC[2]);

        qGrid->gridToCoordOffset[0] = qGrid->extent[0][0];
        qGrid->gridToCoordOffset[1] = qGrid->extent[1][0];
        qGrid->gridToCoordOffset[2] = qGrid->extent[2][0];

        qGrid->gridToCoordScale[0] = 
          (qGrid->extent[0][1] - qGrid->extent[0][0]) / (qGrid->gridDim[0] - 1);
        qGrid->gridToCoordScale[1] = 
          (qGrid->extent[1][1] - qGrid->extent[1][0]) / (qGrid->gridDim[1] - 1);
        qGrid->gridToCoordScale[2] = 
          (qGrid->extent[2][1] - qGrid->extent[2][0]) / (qGrid->gridDim[2] - 1);


        if ( qGrid->gValSpec.sampType == GS_VOLUME)
                {
                getSubsampLocs (&(qGrid->gValSpec), qGrid->gridToCoordScale);
                }



        qGrid->qShape.qc[0] = 1.0 / (qGrid->qShape.ABC[0]*qGrid->qShape.ABC[0]);
        qGrid->qShape.qc[1] = 1.0 / (qGrid->qShape.ABC[1]*qGrid->qShape.ABC[1]);
        qGrid->qShape.qc[2] = 
        qGrid->qShape.qc[3] = 
        qGrid->qShape.qc[4] = 
        qGrid->qShape.qc[5] = 
        qGrid->qShape.qc[6] = 
        qGrid->qShape.qc[7] = 
        qGrid->qShape.qc[8] = 
        qGrid->qShape.qc[9] = 0.0;

        switch (qGrid->qShape.qType)
                {
                case ELLIPSOID:
                        qGrid->qShape.qc[2] = 
                            1.0 / (qGrid->qShape.ABC[2] * qGrid->qShape.ABC[2]);
                        qGrid->qShape.qc[9] = -1;
                        break;

                case ELLIPTIC_PARABOLOID:
                        qGrid->qShape.qc[8] = -1;
                        break;

                case HYPERBOLIC_PARABOLOID:
                        qGrid->qShape.qc[1] = 
                          - 1.0 / (qGrid->qShape.ABC[1] * qGrid->qShape.ABC[1]);
                        qGrid->qShape.qc[8] = -1;
                        break;

                case HYPERBOLOID_ONE_SHEET:
                        qGrid->qShape.qc[2] = 
                          - 1.0 / (qGrid->qShape.ABC[2] * qGrid->qShape.ABC[2]);
                        qGrid->qShape.qc[9] = -1;
                        break;

                case HYPERBOLOID_TWO_SHEETS:
                        qGrid->qShape.qc[2] = 
                          - 1.0 / (qGrid->qShape.ABC[2] * qGrid->qShape.ABC[2]);
                        qGrid->qShape.qc[9] = 1;
                        break;

                default:
                        // should never get here
                        fprintf (stderr,
                          "Bad surface type in getClosestApproachToCurrQ.\n");
                        exit (-1);
                }


        }  // end of deriveQuadParams



int
main (int argc, char **argv)
	{
	QuadricGrid quadricGrid;
        double voxelVal;
        int ijk[3];
        double xyz[3];
        int i, j, k;
        FILE *outFP;

	if (getQuadricParams (argv[1], &quadricGrid))
		{
		exit (-1);
		}

        deriveQuadParams (&quadricGrid);

        if ( (outFP = fopen (quadricGrid.outputFN, "w")) == NULL )
                {
                fprintf (stderr, "Unable to open output file < %s >\n", 
                                quadricGrid.outputFN);
                exit (-1);
                }

        writeVtkHeader  (outFP, &quadricGrid);

        for (k = 0; k < quadricGrid.gridDim[2]; k++)
            {
            for (j = 0; j < quadricGrid.gridDim[1]; j++)
                {
                for (i = 0; i < quadricGrid.gridDim[0]; i++)
                        {
                        ijk[0] = i;
                        ijk[1] = j;
                        ijk[2] = k;

                        ijkToXyz (&quadricGrid, ijk, xyz);

                        if (quadricGrid.gValSpec.sampType == GS_VOLUME)
			        {
                                voxelVal = subsample (&quadricGrid, xyz);
                                }
                        else 
                                {
                                voxelVal = isInside (&quadricGrid, xyz) ? 
                                              quadricGrid.gValSpec.insideVal :
                                              quadricGrid.gValSpec.outsideVal ;
                                }
                        
                        addNoise (quadricGrid.gValSpec.noiseSD, &voxelVal); 

                        writeVtkPoint (outFP, voxelVal);

                        }  // end of k loop
                }  // end of j loop
            }  // end of i loop

        exit (0);

	}  // end of main
