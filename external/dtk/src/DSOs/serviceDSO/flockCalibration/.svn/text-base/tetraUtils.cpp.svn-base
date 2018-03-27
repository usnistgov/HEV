
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// #include "rigidXform.h"

#include "tetraUtils.h"

/*

This source code has some utilities for handling correction
data based on values at vertices of tetrahedra.  
Does interpolation based on tetrahedralization and barycentric coords

*/






#if 0

Here is a simple algorithm for calculating barycentric coordinates
of a point relative to the vertices of a tetrahdron.

This also yields a very simple point-in-tetrahedron test

I got this off the web.


Let the tetrahedron have vertices

        V1 = (x1, y1, z1)
        V2 = (x2, y2, z2)
        V3 = (x3, y3, z3)
        V4 = (x4, y4, z4)

and your test point be

        P = (x, y, z).

Then the point P is in the tetrahedron if following five determinants all have the same sign.

             |x1 y1 z1 1|
        D0 = |x2 y2 z2 1|
             |x3 y3 z3 1|
             |x4 y4 z4 1|

             |x  y  z  1|
        D1 = |x2 y2 z2 1|
             |x3 y3 z3 1|
             |x4 y4 z4 1|

             |x1 y1 z1 1|
        D2 = |x  y  z  1|
             |x3 y3 z3 1|
             |x4 y4 z4 1|

             |x1 y1 z1 1|
        D3 = |x2 y2 z2 1|
             |x  y  z  1|
             |x4 y4 z4 1|

             |x1 y1 z1 1|
        D4 = |x2 y2 z2 1|
             |x3 y3 z3 1|
             |x  y  z  1|

Some additional notes:

    * If by chance the D0=0, then your tetrahedron is 
	degenerate (the points are coplanar).

    * If any other Di=0, then P lies on boundary i 
	(boundary i being that boundary formed by the three 
		points other than Vi).

    * If the sign of any Di differs from that of D0 then P is 
	outside boundary i.

    * If the sign of any Di equals that of D0 then P is inside boundary i.

    * If P is inside all 4 boundaries, then it is inside the tetrahedron.

    * As a check, it must be that D0 = D1+D2+D3+D4.

    * The pattern here should be clear; the computations can be extended 
	to simplicies of any dimension. (The 2D and 3D case are the 
	triangle and the tetrahedron).

    * If it is meaningful to you, the quantities bi = Di/D0 are the 
	usual barycentric coordinates.

    * Comparing signs of Di and D0 is only a check that P and Vi are 
	on the same side of boundary i. 


#endif






static double
det2v (double *v0, double *v1)
	{
	return v0[0]*v1[1] - v1[0]*v0[1];
	}

static double
det3v (double *v0, double *v1, double *v2)
	{
	double m0, m1, m2;

	m0 = det2v (v1, v2);
	m1 = det2v (v0, v2);
	m2 = det2v (v0, v1);
	return v0[2]*m0 - v1[2]*m1 + v2[2]*m2;
	}




static double
det4v_h (double *v0, double *v1, double *v2, double *v3)
	{
	// assume homogeneous coords with last coord = 1
	// we use last col to calc minors

	double m0, m1, m2, m3;

	m0 = det3v (v1, v2, v3);
	m1 = det3v (v0, v2, v3);
	m2 = det3v (v0, v1, v3);
	m3 = det3v (v0, v1, v2);

	return -m0 + m1 - m2 + m3;
	}  // end of det4v






static void
getBarycentricCoords ( 	double pt[3], 
			Tetrahedron *tetra, 
			double baryCoord[4] )
	{
	double detFull, det0, det1, det2, det3;


	detFull = det4v_h (tetra->v[0], tetra->v[1], tetra->v[2], tetra->v[3]);
	det0    = det4v_h (pt,          tetra->v[1], tetra->v[2], tetra->v[3]);
	det1    = det4v_h (tetra->v[0], pt,          tetra->v[2], tetra->v[3]);
	det2    = det4v_h (tetra->v[0], tetra->v[1], pt,          tetra->v[3]);
	det3    = det4v_h (tetra->v[0], tetra->v[1], tetra->v[2], pt         );

	baryCoord[0] = det0 / detFull;
	baryCoord[1] = det1 / detFull;
	baryCoord[2] = det2 / detFull;
	baryCoord[3] = det3 / detFull;

	}  // end of getBarycentricCoords




static
Tetrahedron *containingTetraSearch (	double pt[3], 
					Tetrahedron *startTetra, 
					double baryCoord[4])
	{
	Tetrahedron *currTetra;
	int minCoordI;
	int i;

	currTetra = startTetra;

	while (currTetra != NULL)
		{
		getBarycentricCoords (pt, currTetra, baryCoord);

		for (i = 0; i < 4; i++)
			{
			if (baryCoord[i] < 0)
				{
				if (baryCoord[i] > -0.0001)
					{
					baryCoord[i] = 0;
					}
				}
			}

		minCoordI = 0;
		if (baryCoord[1] < baryCoord[0])
			{
			minCoordI = 1;
			}

		if (baryCoord[2] < baryCoord[minCoordI])
			{
			minCoordI = 2;
			}

		if (baryCoord[3] < baryCoord[minCoordI])
			{
			minCoordI = 3;
			}

		if (baryCoord[minCoordI] >= 0)
			{
			break;
			}

		currTetra = currTetra->neighbor[minCoordI];
		}

	return currTetra;
	
	}  // end of getBarycentricCoords





static int
setupTetraList (	int numVertices,
			Vertex *vertices,
			int numTetrahedra,
			TetraIndices tetraIndices[],
			TetrahedronList *tetraList)
	{
	int i, j;

	tetraList->vertexList.numVertices = numVertices;
	tetraList->vertexList.vertex = 
		(Vertex *) malloc ((numVertices) * sizeof (Vertex));
	if (tetraList->vertexList.vertex == NULL)
		{
		return -2;
		}
	memcpy (tetraList->vertexList.vertex, 
			vertices, 3 * numVertices * sizeof (double));


	tetraList->numTetrahedra = numTetrahedra;
	tetraList->tetrahedron = 
		(Tetrahedron *) malloc (numTetrahedra * sizeof (Tetrahedron));
	if (tetraList->tetrahedron == NULL)
		{
		return -3;
		}

	for (i = 0; i < numTetrahedra; i++)
	  {
	  for (j = 0; j < 4; j++)
	    {
	    tetraList->tetrahedron[i].vIndex[j] = tetraIndices[i][j];
	    tetraList->tetrahedron[i].v[j] = 
			tetraList->vertexList.vertex[tetraIndices[i][j]];
	    tetraList->tetrahedron[i].neighbor[j] = NULL;
	    }
	  }

	tetraList->currentTetra = &(tetraList->tetrahedron[0]);

	return 0;
	}   // end of setupTetraList




static void
sortVertexList (Tetrahedron *tetra)
	{
	int i, j, tmpI;
	double *tmpV;

	// sort by index

	// simple bubble sort; it doesn't have to be fast

	for (j = 2; j >= 0; j--)
		{
		for (i = j; i < 3; i++)
			{
			if (tetra->vIndex[i] > tetra->vIndex[i+1])
				{
				// Note the redunant encoding of vertex ptr
				tmpV = tetra->v[i+1];
				tetra->v[i+1] = tetra->v[i];
				tetra->v[i] = tmpV;
				tmpI = tetra->vIndex[i+1];
				tetra->vIndex[i+1] = tetra->vIndex[i];
				tetra->vIndex[i] = tmpI;
				}
			}
		}

	return;
	}  // sortVertexList


static void
sortTetraVertices (TetrahedronList *tetraList)
	{
	int i;

	for (i = 0; i < tetraList->numTetrahedra; i++)
		{
		sortVertexList (&(tetraList->tetrahedron[i]));
		}

	return;
	}  // end of sortTetraVertices


static int
findFaceIndex (int *vertIndex)
	{
	int i, j, vertexFound;


	for (i = 0; i < 4; i++)
		{

		vertexFound = 0;

		for (j = 0; j < 3; j++)
			{
			if (i == vertIndex[j])
				{
				vertexFound = 1;
				break;
				}
			}

		if ( ! vertexFound )
			{
			return i;
			}

		}
	
	// we really can't get here
	return 0;
	}  // end of findFaceIndex


static void
findCommonFaces (Tetrahedron *a, Tetrahedron *b)
	{
	int aCommonVerts[3];
	int bCommonVerts[3];
	int i, j;
	int numCommonVerts;
	int aFace, bFace;

	if (a->vIndex[0] > b->vIndex[3])
		{
		return;
		}

	if (a->vIndex[3] < b->vIndex[0])
		{
		return;
		}


	numCommonVerts = 0;

	for (i = 0; i < 4; i++)
		{
		for (j = 0; j < 4; j++)
			{
			if (a->vIndex[i] == b->vIndex[j])
				{
				aCommonVerts[numCommonVerts] = i;
				bCommonVerts[numCommonVerts] = j;
				numCommonVerts++;
				}

			}
		}

	if (numCommonVerts == 3)
		{
		aFace = findFaceIndex (aCommonVerts);
		bFace = findFaceIndex (bCommonVerts);
		a->neighbor[aFace] = b;
		b->neighbor[bFace] = a;
		}

	return;
	}   // end of findCommonFaces


static void
findAllCommonFaces (TetrahedronList *tetraList)
	{

	int i, j;

	for (i = 0; i < tetraList->numTetrahedra; i++)
		{
		for (j = i+1; j < tetraList->numTetrahedra; j++)
			{

			// printf ("findAllCommonFaces: %d %d\n", i, j);

			findCommonFaces (
				&(tetraList->tetrahedron[i]), 
				&(tetraList->tetrahedron[j])  );
			}
		}

	}  // end of findAllCommonFaces





static int
buildTetrahedra (	int numVertices,
			Vertex *vertices,
			int numTetrahedra,
			TetraIndices tetraIndices[],
			TetrahedronList *tetraList)
	{

	setupTetraList (numVertices, vertices, 
				numTetrahedra, tetraIndices, tetraList);


	sortTetraVertices (tetraList);

	findAllCommonFaces (tetraList);

	return 0;
	}  // end of buildTetrahedra

int 
tetraInterpReal (	TetrahedronList *tetraList, 
			double xyz[3], 
			double vertexVal[], 
			double *interpolatedVal )
	{

	Tetrahedron *containingTet;
	double baryCoord[4];

	containingTet = 
		containingTetraSearch (xyz, tetraList->currentTetra, baryCoord);

	if (containingTet == NULL)
		{
		*interpolatedVal = 0;
		return -1;
		}

	*interpolatedVal =
		baryCoord[0]*vertexVal[containingTet->vIndex[0]] +
		baryCoord[1]*vertexVal[containingTet->vIndex[1]] +
		baryCoord[2]*vertexVal[containingTet->vIndex[2]] +
		baryCoord[3]*vertexVal[containingTet->vIndex[3]] ;

	tetraList->currentTetra = containingTet;

	return 0;
	}  // end of tetraInterpReal

int 
tetraInterpReal3 (	TetrahedronList *tetraList, 
			double xyz[3], 
			double vertexVal[][3], 
			double interpolatedVal[3])
	{

	Tetrahedron *containingTet;
	double baryCoord[4];

#if 0
	printf ("\n\ntetraInterpReal3\n");
	printf ("xyz = %f %f %f\n", xyz[0], xyz[1], xyz[2]);
#endif

	containingTet = 
		containingTetraSearch (xyz, tetraList->currentTetra, baryCoord);

#if 0
	{
	int i, j;
	printf ("containing tet = %p\n", containingTet);
	for (j = 0; j < 4; j++)
		{
		printf ("v %d  index %i   weight %f  v[%d] = %f %f %f\n", 
			j,
			containingTet->vIndex[j],
			baryCoord[j],
			containingTet->vIndex[j],
			vertexVal[containingTet->vIndex[j]][0],
			vertexVal[containingTet->vIndex[j]][1],
			vertexVal[containingTet->vIndex[j]][2]);
			
		}
	}
#endif


	if (containingTet == NULL)
		{
		interpolatedVal[0] = 0;
		interpolatedVal[1] = 0;
		interpolatedVal[2] = 0;
		return -1;
		}

	interpolatedVal[0] =
		baryCoord[0]*vertexVal[containingTet->vIndex[0]][0] +
		baryCoord[1]*vertexVal[containingTet->vIndex[1]][0] +
		baryCoord[2]*vertexVal[containingTet->vIndex[2]][0] +
		baryCoord[3]*vertexVal[containingTet->vIndex[3]][0] ;

	interpolatedVal[1] =
		baryCoord[0]*vertexVal[containingTet->vIndex[0]][1] +
		baryCoord[1]*vertexVal[containingTet->vIndex[1]][1] +
		baryCoord[2]*vertexVal[containingTet->vIndex[2]][1] +
		baryCoord[3]*vertexVal[containingTet->vIndex[3]][1] ;

	interpolatedVal[2] =
		baryCoord[0]*vertexVal[containingTet->vIndex[0]][2] +
		baryCoord[1]*vertexVal[containingTet->vIndex[1]][2] +
		baryCoord[2]*vertexVal[containingTet->vIndex[2]][2] +
		baryCoord[3]*vertexVal[containingTet->vIndex[3]][2] ;

#if 0
	printf ("interpolatedVal = %f %f %f\n",
		interpolatedVal[0],
		interpolatedVal[1],
		interpolatedVal[2]);
	printf ("\n\n");
#endif

	tetraList->currentTetra = containingTet;

	return 0;
	}  // end of tetraInterpReal3




#include "SWA/SphereMean/sphericalWeightedAverage.h"

int 
tetraInterpQuatSWA (	TetrahedronList *tetraList, 
			double xyz[3], 
			Quat quat[], 
			Quat interpolatedQuat)
	{

	// This routine could easily be generalized to do a weighted
	// average of any number of rotational quaternions

	Tetrahedron *containingTet;
	double baryCoord[4];
	double tetQuat[4][4];

	containingTet = 
		containingTetraSearch (xyz, tetraList->currentTetra, baryCoord);

	if (containingTet == NULL)
		{
		interpolatedQuat[0] = 0;
		interpolatedQuat[1] = 0;
		interpolatedQuat[2] = 0;
		interpolatedQuat[3] = 1;
		return -1;
		}

	memcpy (tetQuat[0], quat[containingTet->vIndex[0]], sizeof(Quat));
	memcpy (tetQuat[1], quat[containingTet->vIndex[1]], sizeof(Quat));
	memcpy (tetQuat[2], quat[containingTet->vIndex[2]], sizeof(Quat));
	memcpy (tetQuat[3], quat[containingTet->vIndex[3]], sizeof(Quat));

	sphericalWeightedAverage4 (4, tetQuat, baryCoord, interpolatedQuat);

	return 0;
	}  // end of tetraInterpQuatSWA




static int
getDataLine (char *line, int n, FILE *fp)
	{
	char token[2000];

	while (1)
		{
		if (fgets (line, n, fp) == NULL)
			{
			return -1;
			}

		if (sscanf (line, "%s", token) == 1)
			{
			if (token[0] != '#')
				{
				return 0;
				}
			}
		}

	return -2;
	}	// end of getDataLine


int
readCorrectionData (	const char *correctionDataFN, 
			int *numDataPts, 
			Vertex **trueXYZ, 
			Quat **correctionRot,
			TetrahedronList *tetraList )
	{
	int numVertices;
	Vertex *vertices;
	int numTetrahedra;
	// int *tetraIndices;
	TetraIndices *tetraIndices;

	FILE *inFP;
	int i;
	char line[1000];

	if ( (inFP = fopen (correctionDataFN, "r")) == NULL)
		{
		return -1;
		}

	if (getDataLine (line, sizeof(line), inFP))
		{
		fclose (inFP);
		return -2;
		}

	if (sscanf (line, "%d %d", &numVertices, &numTetrahedra) != 2)
		{
		fclose (inFP);
		return -3;
		}

	if ( (vertices = (Vertex *) 
			malloc (numVertices * sizeof (Vertex))) == NULL)
		{
		return 1;
		}

	if ( (tetraIndices = (TetraIndices *) 
			malloc (numTetrahedra * sizeof (TetraIndices))) == NULL)
		{
		return 2;
		}

	if ( (*trueXYZ = (Vertex *) 
			malloc (numVertices * sizeof (Vertex))) == NULL)
		{
		return 3;
		}

	if ( (*correctionRot = (Quat *) 
			malloc (numVertices * sizeof (Quat))) == NULL)
		{
		return 4;
		}



	for (i = 0; i < numVertices; i++)
		{
		if (getDataLine (line, sizeof(line), inFP))
			{
			fclose (inFP);
			return -4;
			}
		if (sscanf (line, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
			(*trueXYZ)[i]+0,
			(*trueXYZ)[i]+1,
			(*trueXYZ)[i]+2,
			vertices[i]+0,
			vertices[i]+1,
			vertices[i]+2,
			(*correctionRot)[i]+0,
			(*correctionRot)[i]+1,
			(*correctionRot)[i]+2,
			(*correctionRot)[i]+3  ) != 10)
			{
			fclose (inFP);
			return -5;
			}
		}

	for (i = 0; i < numTetrahedra; i++)
		{
		if (getDataLine (line, sizeof(line), inFP))
			{
			fclose (inFP);
			return -6;
			}
		if (sscanf (line, "%d %d %d %d",
			tetraIndices[i]+0,
			tetraIndices[i]+1,
			tetraIndices[i]+2,
			tetraIndices[i]+3 ) != 4)
			{
			fclose (inFP);
			return -7;
			}
		}

	fclose (inFP);

	if (buildTetrahedra (numVertices, vertices, 
				numTetrahedra, tetraIndices, tetraList))
		{
		return -8;
		}



	return 0;
	}  //  end of readCorretionData




