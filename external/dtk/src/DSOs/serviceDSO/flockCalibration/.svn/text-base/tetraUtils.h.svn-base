
/*

Some utilities for handling correction
data based on values at vertices of tetrahedra.  
Does interpolation based on tetrahedralization and barycentric coords

*/

typedef double Vertex[3];
typedef double Quat[4];
typedef int TetraIndices[4];

typedef struct tetrahedron
	{
	double *v[4];
	int vIndex[4];
	struct tetrahedron *neighbor[4];
	}  Tetrahedron;

typedef struct vertex_list
	{
	int numVertices;
	Vertex *vertex;
	} VertexList;

typedef struct tetrahedron_list
	{
	int numTetrahedra;
	Tetrahedron *tetrahedron;
	VertexList vertexList;
	Tetrahedron *currentTetra;
	} TetrahedronList;






int 
tetraInterpReal (	TetrahedronList *tetraList, 
			double xyz[3], 
			double vertexVal[], 
			double *interpolatedVal );

int 
tetraInterpReal3 (	TetrahedronList *tetraList, 
			double xyz[3], 
			double vertexVal[][3], 
			double interpolatedVal[3]);

int
tetraInterpQuatSWA (	TetrahedronList *tetraList, 
			double xyz[3], 
			Quat quat[], 
			Quat interpolatedQuat);

int
readCorrectionData (	const char *correctionDataFN, 
			int *numDataPts, 
			Vertex **trueXYZ, 
			Quat **correctionRot,
			TetrahedronList *tetraList );



