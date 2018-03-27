
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gle.h"
#include "vvector.h"

////////////////////////////////////////////////////////////////////
//
//

#define MAX_MATRIX 1000
#define MAX_VERT   10000

#define NO_STATE   0
#define TMESH_STATE  1
#define POLYGON_STATE  2

static int CurrentState = NO_STATE;

typedef double Matrix[4][4];

static Matrix MatrixStack[MAX_MATRIX];
static int CurrentMatrix = -1;

static int NoNorm = 1;
static double CurrentNormal[3];
static double CurrentRGBA[4];

static int NumVerts = 0;
static double VertexNormList[MAX_VERT][6];

/////

#define OUTPUT_FORMAT_SAVG  ('s')
#define OUTPUT_FORMAT_OSG   ('o')

static int OutputFormat = OUTPUT_FORMAT_OSG;

//
//
////////////////////////////////////////////////////////////////////

int
setExtrusionOutputFormat (int format)
	{

	if ( (format != OUTPUT_FORMAT_SAVG) && (format != OUTPUT_FORMAT_OSG))
		{
		return -1;
		}

	OutputFormat = format;
	return 0;
	}
		

static void
writeSavgComment (char *comment)
	{
	char localComment[5000];
	char *lc;
	char *line;

	strcpy (localComment, comment);

	lc = localComment;
	while ( (line = strtok (lc, "\n")) != NULL)
		{
		lc = NULL;
		printf ("# %s\n", line);
		}
	printf ("\n");
	}  // end of writeSavgComment

static void
writeOsgComment (char *comment)
	{
	printf ("Comment {\n\n");
	printf ("%s\n", comment);
	printf ("}\n");
	}  // end of writeOsgComment


void
writeExtrusionComment (char *comment)
	{

	if (OutputFormat == OUTPUT_FORMAT_SAVG)
		{
		writeSavgComment (comment);
		}
	else
		{
		writeOsgComment (comment);
		}

	return;
	}  // end of writeComment








static void
setupFirstMatrixIfNecessary ()
	{
	if (CurrentMatrix == -1)
		{
		IDENTIFY_MATRIX_4X4 (MatrixStack[0]);
		CurrentMatrix = 0;
		}

	}  // end of setupFirstMatrixIfNecessary


static int
checkMatrix (Matrix mat)
	{
	double rowLen[3], colLen[3];
	double rowDot[3], colDot[3];
	int i, j, next;
	double det;

#if 0
	DETERMINANT_3X3 (det, mat);
	fprintf (stderr, "Det = %f\n", det);
#endif

	for (i = 0; i < 3; i++)
		{
		rowLen[i] = 0;
		colLen[i] = 0;
		colDot[i] = 0;
		rowDot[i] = 0;
		next = (i + 1) % 3;
		for (j = 0; j < 3; j++)
			{
			rowLen[i] += mat[i][j]*mat[i][j];
			colLen[i] += mat[j][i]*mat[j][i];
			rowDot[i] += mat[i][j]*mat[next][j];
			colDot[i] += mat[j][i]*mat[j][next];
			}

		}

	for (i = 0; i < 3; i++)
		{

#if 0
		fprintf (stderr, "rowLen[%d] = %f\n", i, rowLen[i]);
		fprintf (stderr, "colLen[%d] = %f\n", i, colLen[i]);
		fprintf (stderr, "rowDot[%d] = %f\n", i, rowDot[i]);
		fprintf (stderr, "colDot[%d] = %f\n", i, colDot[i]);
#endif


		if ( (rowLen[i] < 0.99) || (rowLen[i] > 1.01) )
			{
			fprintf (stderr, "Error in rowLen[%d] \n", rowLen[i]);
			return -1;
			}

		if ( (colLen[i] < 0.99) || (colLen[i] > 1.01) )
			{
			fprintf (stderr, "Error in colLen[%d] \n", colLen[i]);
			return -1;
			}

		if ( (rowDot[i] < -0.01) || (rowDot[i] > 0.01) )
			{
			fprintf (stderr, "Error in rowDot[%d] \n", rowDot[i]);
			return -1;
			}

		if ( (colDot[i] < -0.01) || (colDot[i] > 0.01) )
			{
			fprintf (stderr, "Error in colDot[%d] \n", colDot[i]);
			return -1;
			}



		}

	return 0;
	}  // end of checkMatrix

int BGNTMESH(int i, double len)
	{
	NumVerts = 0;
	CurrentState = TMESH_STATE;

	
	return 0;
	}  // end of BGNTMESH


static int
endOsgPolyGroupInner (char *primitiveType)
	{
	// It turns out that the only difference between the triangle
	// mesh and the polygon OSG output is the name of the 
	// primitive that is put in the OSG file.  So I'm combining 
	// the code for these two cases.

	int i;


	printf ("Group {\n");
	printf ("  num_children 1\n");


	printf ("  Geode {\n");
	printf ("    DataVariance STATIC\n");
	printf ("    num_drawables 1\n");
	printf ("    Geometry {\n");
	printf ("      DataVariance STATIC\n");
	printf ("      useDisplayList TRUE\n");
	printf ("      useVertexBufferObjects FALSE\n");
	printf ("      PrimitiveSets 1\n");
	printf ("      {\n");
	printf ("        DrawArrays %s 0 %d\n", primitiveType, NumVerts);
	printf ("      }\n");
	printf ("      VertexArray Vec3Array %d\n", NumVerts);
	printf ("      {\n");

	for (i = 0; i < NumVerts; i++)
		{
		printf ("        %lf %lf %lf\n", 
				VertexNormList[i][0],
				VertexNormList[i][1],
				VertexNormList[i][2]);
		}
		



	printf ("      }\n");   // end of VertexArray
	printf ("      NormalBinding PER_VERTEX\n");
	printf ("      NormalArray Vec3Array %d\n", NumVerts);
	printf ("      {\n");


	for (i = 0; i < NumVerts; i++)
		{
		printf ("        %lf %lf %lf\n", 
				VertexNormList[i][3],
				VertexNormList[i][4],
				VertexNormList[i][5]);
		}


	printf ("      }\n");  // end of NormalArray
	printf ("    }\n");  // end of geom
	printf ("  }\n");  // end of geode



	printf ("}\n");  // end of group

	CurrentState = NO_STATE;

	return 0;
	}  // end of endOsgPolyGroupInner 


static int
endSavgTMesh ()
	{
	int i, j;
	

	for (i = 0; i < (NumVerts-2); i += 2)
		{
		printf ("polygon\n");
		for (j = 0; j < 3; j++)
			{
			printf (" %lf %lf %lf %lf %lf %lf\n", 
				VertexNormList[i+j][0],
				VertexNormList[i+j][1],
				VertexNormList[i+j][2],
				VertexNormList[i+j][3],
				VertexNormList[i+j][4],
				VertexNormList[i+j][5]
				);
			}

		if ((i+3) < NumVerts)
		    {
		    printf ("polygon\n");
		    for (j = 2; j >= 0; j--)
			{
			printf (" %lf %lf %lf %lf %lf %lf\n", 
				VertexNormList[1+i+j][0],
				VertexNormList[1+i+j][1],
				VertexNormList[1+i+j][2],
				VertexNormList[1+i+j][3],
				VertexNormList[1+i+j][4],
				VertexNormList[1+i+j][5]
				);
			}
		    }

		}
	printf ("\n");



	}  // end of endSavgTMesh


static int
endSavgPolygon ()
	{
	int i;

	printf ("polygon\n");
	for (i = 0; i < NumVerts; i++)
		{
		printf (" %lf %lf %lf %lf %lf %lf\n", 
			VertexNormList[i][0],
			VertexNormList[i][1],
			VertexNormList[i][2],
			VertexNormList[i][3],
			VertexNormList[i][4],
			VertexNormList[i][5]
			);
		}
	printf ("\n");


	return 0;
	}  // end of endSavgPolygon

#if 1

int ENDTMESH(void)
	{
	if (NumVerts < 3)
		{
		return 0;
		}


	if (OutputFormat == OUTPUT_FORMAT_SAVG)
		{
		return endSavgTMesh ();
		}

	return endOsgPolyGroupInner ("TRIANGLE_STRIP");
	}  // end of ENDTMESH

int ENDPOLYGON(void)
	{
	if (NumVerts < 3)
		{
		return 0;
		}

	if (OutputFormat == OUTPUT_FORMAT_SAVG)
		{
		return endSavgPolygon ();
		}

	return endOsgPolyGroupInner ("TRIANGLE_FAN");
	}  // end of ENDPOLYGON

#else

int ENDTMESH(void)
	{

	int i;


	printf ("Group {\n");
	printf ("  num_children 1\n");


	printf ("  Geode {\n");
	printf ("    DataVariance STATIC\n");
	printf ("    num_drawables 1\n");
	printf ("    Geometry {\n");
	printf ("      DataVariance STATIC\n");
	printf ("      useDisplayList TRUE\n");
	printf ("      useVertexBufferObjects FALSE\n");
	printf ("      PrimitiveSets 1\n");
	printf ("      {\n");
	printf ("        DrawArrays TRIANGLE_STRIP 0 %d\n", NumVerts);
	printf ("      }\n");
	printf ("      VertexArray Vec3Array %d\n", NumVerts);
	printf ("      {\n");

	for (i = 0; i < NumVerts; i++)
		{
		printf ("        %lf %lf %lf\n", 
				VertexNormList[i][0],
				VertexNormList[i][1],
				VertexNormList[i][2]);
		}




	printf ("      }\n");   // end of VertexArray
	printf ("      NormalBinding PER_VERTEX\n");
	printf ("      NormalArray Vec3Array %d\n", NumVerts);
	printf ("      {\n");


	for (i = 0; i < NumVerts; i++)
		{
		printf ("        %lf %lf %lf\n", 
				VertexNormList[i][3],
				VertexNormList[i][4],
				VertexNormList[i][5]);
		}


	printf ("      }\n");  // end of NormalArray
	printf ("    }\n");  // end of geom
	printf ("  }\n");  // end of geode



	printf ("}\n");  // end of group

	CurrentState = NO_STATE;

	return 0;
	}  // end of ENDTMESH


int ENDPOLYGON(void)
	{

	int i;


	printf ("Group {\n");
	printf ("  num_children 1\n");


	printf ("  Geode {\n");
	printf ("    DataVariance STATIC\n");
	printf ("    num_drawables 1\n");
	printf ("    Geometry {\n");
	printf ("      DataVariance STATIC\n");
	printf ("      useDisplayList TRUE\n");
	printf ("      useVertexBufferObjects FALSE\n");
	printf ("      PrimitiveSets 1\n");
	printf ("      {\n");
	// printf ("        DrawArrays POLYGON 0 %d\n", NumVerts);
	printf ("        DrawArrays TRIANGLE_FAN 0 %d\n", NumVerts);
	printf ("      }\n");
	printf ("      VertexArray Vec3Array %d\n", NumVerts);
	printf ("      {\n");

	for (i = 0; i < NumVerts; i++)
		{
		printf ("        %lf %lf %lf\n", 
				VertexNormList[i][0],
				VertexNormList[i][1],
				VertexNormList[i][2]);
		}




	printf ("      }\n");   // end of VertexArray
	printf ("      NormalBinding PER_VERTEX\n");
	printf ("      NormalArray Vec3Array %d\n", NumVerts);
	printf ("      {\n");


	for (i = 0; i < NumVerts; i++)
		{
		printf ("        %lf %lf %lf\n", 
				VertexNormList[i][3],
				VertexNormList[i][4],
				VertexNormList[i][5]);
		}


	printf ("      }\n");  // end of NormalArray
	printf ("    }\n");  // end of geom
	printf ("  }\n");  // end of geode



	printf ("}\n");  // end of group

	CurrentState = NO_STATE;

	return 0;
	}  // end of ENDPOLYGON
#endif



















int BGNPOLYGON(void)
	{
	NumVerts = 0;
	CurrentState = POLYGON_STATE;

	return 0;
	}  // end of BGNPOLYGON


#if 0
int ENDPOLYGON(void)
	{


	fprintf (stderr, "Can't handle polygons yet.\n");

	CurrentState = NO_STATE;
	}  // end of ENDPOLYGON
#endif


int V3F_F(float *x, int j, int id)
	{
	double d[3];

	d[0] = x[0];
	d[1] = x[1];
	d[2] = x[2];

	return V3F_D (d, j, id);
	
	}  // end of V3F_F

static void
multPtByMatrix (double ptIn[3], Matrix mat, double ptOut[3])
	{
	int i, j;

	for (i = 0; i < 3; i++)
		{
		ptOut[i] = 0;
		for (j = 0; j < 3; j++)
			{
			ptOut[i] += ptIn[j]*(mat)[j][i];
			}
		ptOut[i] += (mat)[3][i];
		}

	}  // end of multPtByMatrix

multPtByMatrix_3X3 (double ptIn[3], Matrix mat, double ptOut[3])
	{
	int i, j;

	for (i = 0; i < 3; i++)
		{
		ptOut[i] = 0;
		for (j = 0; j < 3; j++)
			{
			ptOut[i] += ptIn[j]*(mat)[j][i];
			}
		}

	}  // end of multPtByMatrix_3X3





int V3F_D(double *x, int j, int id)
	{


	if (NumVerts == (MAX_VERT-1))
		{
		return -1;
		}

	if (NoNorm)
		{
		CurrentNormal[0] = 1;
		CurrentNormal[1] = 0;
		CurrentNormal[2] = 0;
		NoNorm = 0;
		}




	// Need to transform x and curr normal 
	multPtByMatrix (x, MatrixStack[CurrentMatrix], 
				&(VertexNormList[NumVerts][0]));
	multPtByMatrix_3X3 (CurrentNormal, MatrixStack[CurrentMatrix], 
				&(VertexNormList[NumVerts][3]));


	NumVerts++;
	
	return 0;
	}  // end of V3F_D


int N3F_F(float *x)
	{
	CurrentNormal[0] = x[0];
	CurrentNormal[1] = x[1];
	CurrentNormal[2] = x[2];

	NoNorm = 0;

	return 0;
	}  // end of N3F_F


int N3F_D(double *x)
	{
	CurrentNormal[0] = x[0];
	CurrentNormal[1] = x[1];
	CurrentNormal[2] = x[2];

	NoNorm = 0;


	return 0;
	}  // end of N3F_D


int C3F(float *x)
	{
	// do nothing
	return 0;
	}  // end of C3F


int T2F_F(float x, double y)
	{
	// do nothing
	return 0;
	}  // end of T2F_F


int T2F_D(double x, double y)
	{
	// do nothing
	return 0;
	}  // end of T2F_D



int POPMATRIX(void)
	{

	setupFirstMatrixIfNecessary ();

	if (CurrentMatrix == 0)
		{
		IDENTIFY_MATRIX_4X4 (MatrixStack[0]);
		}
	else
		{
		CurrentMatrix--;
		}

	return 0;
	}  // end of POPMATRIX


int PUSHMATRIX(void)
	{

	setupFirstMatrixIfNecessary ();

	if (CurrentMatrix < (MAX_MATRIX-1))
		{
		memcpy (MatrixStack[CurrentMatrix+1] , MatrixStack[CurrentMatrix], sizeof (Matrix));
		CurrentMatrix++;
		return 0;
		}

	return 1;
	}  // end of PUSHMATRIX


int MULTMATRIX_F(float *x)
	{
	int i, j;
	Matrix m;

	setupFirstMatrixIfNecessary ();

	for (i = 0; i < 4; i++)
		{
		for (j = 0; j < 4; j++)
			{
			m[i][j] = *(x++);
			}
		}

	return MULTMATRIX_D (m);
	}  // end of MULTMATRIX


int LOADMATRIX_F(float *x)
	{
	int i, j;

	setupFirstMatrixIfNecessary ();


	for (i = 0; i < 4; i++)
		{
		for (j = 0; j < 4; j++)
			{
			MatrixStack[CurrentMatrix][i][j] = *(x++);
			}
		}

	return checkMatrix (MatrixStack[CurrentMatrix]);
	}  // end of LOADMATRIX


int LOADMATRIX_D(double *x)
	{
	int i, j;

	setupFirstMatrixIfNecessary ();


	for (i = 0; i < 4; i++)
		{
		for (j = 0; j < 4; j++)
			{
			MatrixStack[CurrentMatrix][i][j] = *(x++);
			}
		}

	return checkMatrix (MatrixStack[CurrentMatrix]);
	}  // end of LOADMATRIX_D


int MULTMATRIX_D(double x[4][4])
	{


	setupFirstMatrixIfNecessary ();

	Matrix result;
	MATRIX_PRODUCT_4X4 (result, x, MatrixStack[CurrentMatrix]);

	// MatrixStack[CurrentMatrix] = result;
	memcpy (MatrixStack[CurrentMatrix] , result, sizeof (Matrix));

#if 0
	{
	int i, j;
	for (i = 0; i < 4; i++)
		{
		fprintf (stderr, "  matrix row %d :   ", i);
		for (j = 0; j < 4; j++)
			{
			fprintf (stderr, 
				"  %lf ", MatrixStack[CurrentMatrix][i][j]);
			}
			fprintf (stderr, "\n");
		}
	}
#endif



	return checkMatrix (MatrixStack[CurrentMatrix]);
	}  // end of MULTMATRIX_D



