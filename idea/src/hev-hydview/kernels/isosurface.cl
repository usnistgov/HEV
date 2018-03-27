#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable
#define NTHREADS 32
#define MAX_VERTS_PER_CELL 15

//how to sample "images"
sampler_t gradientSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
sampler_t volumeSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
sampler_t tableSampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

// compute interpolated vertex along an edge
float4 vertexInterp(float isolevel, float4 p0, float4 p1, float f0, float f1)
{
    float t = (isolevel - f0) / (f1 - f0);
	return mix(p0, p1, t);
} 

//generate gradient from volume
__kernel
void
gradient(__read_only image3d_t volume, __global float4 *grad_array, uint4 gridSize)
{

    uint i = get_global_id(0);

    if(i >= gridSize.x*gridSize.y*gridSize.z) { return; }

    int4 gridPos;
    gridPos.z = i / (gridSize.x*gridSize.y);
    gridPos.y = (i - gridPos.z*gridSize.x*gridSize.y)/gridSize.x;
    gridPos.x = i - gridPos.z*gridSize.x*gridSize.y - gridPos.y*gridSize.x;
    gridPos.w = 1.0;

    //gradient = (neighbor2 - neighbor1)/distance
    int4 neighbor1, neighbor2; 
    float distance;
    float4 grad;

    //x direction
    if(gridPos.x >= gridSize.x) {
        neighbor2 = (int4)(0, 0, 0, 0);
        neighbor1 = (int4)(-1, 0, 0, 0);
        distance = 1;
    } else if(gridPos.x <= 0) {
        neighbor2 = (int4)(1, 0, 0, 0);
        neighbor1 = (int4)(0, 0, 0, 0);
        distance = 1;
    } else {
        neighbor2 = (int4)(1, 0, 0, 0);
        neighbor1 = (int4)(-1, 0, 0, 0);
        distance = 2;
    }
    grad.x = (read_imagef(volume, volumeSampler, gridPos + neighbor2).x - 
              read_imagef(volume, volumeSampler, gridPos + neighbor1).x)/distance;
    //y direction
    if(gridPos.y >= gridSize.y) {
        neighbor2 = (int4)(0, 0, 0, 0);
        neighbor1 = (int4)(0, -1, 0, 0);
        distance = 1;
    } else if(gridPos.y <= 0) {
        neighbor2 = (int4)(0, 1, 0, 0);
        neighbor1 = (int4)(0, 0, 0, 0);
        distance = 1;
    } else {
        neighbor2 = (int4)(0, 1, 0, 0);
        neighbor1 = (int4)(0, -1, 0, 0);
        distance = 2;
    }
    grad.y = (read_imagef(volume, volumeSampler, gridPos + neighbor2).x - 
              read_imagef(volume, volumeSampler, gridPos + neighbor1).x)/distance;
    //z direction
    if(gridPos.z >= gridSize.z) {
        neighbor2 = (int4)(0, 0, 0, 0);
        neighbor1 = (int4)(0, 0, -1, 0);
        distance = 1;
    } else if(gridPos.z <= 0) {
        neighbor2 = (int4)(0, 0, 1, 0);
        neighbor1 = (int4)(0, 0, 0, 0);
        distance = 1;
    } else {
        neighbor2 = (int4)(0, 0, 1, 0);
        neighbor1 = (int4)(0, 0, -1, 0);
        distance = 2;
    }
    grad.z = (read_imagef(volume, volumeSampler, gridPos + neighbor2).x - 
              read_imagef(volume, volumeSampler, gridPos + neighbor1).x)/distance;

    //write result into image3D
    grad.w = 0.0;
    
    grad_array[i] = grad;

}

//generate triangles from marching cubes and normals from gradient
__kernel
void
isosurface(__global float4 *triangles, __global float4 *normals,
           __read_only image3d_t volume, __read_only image3d_t gradient,
           uint4 gridSize, uint numVoxels, float isoValue,
           __read_only image2d_t numVertsTex, __read_only image2d_t triTex)
{
    //uint blockId = get_group_id(0);
    uint i = get_global_id(0);
    uint tid = get_local_id(0);

    if(i >= numVoxels) { return; }

    int4 gridBounds;
    gridBounds.x = gridSize.x-1;
    gridBounds.y = gridSize.y-1;
    gridBounds.z = gridSize.z-1;

    int4 gridPos;
    gridPos.z = i / (gridBounds.x*gridBounds.y);
    gridPos.y = (i - gridPos.z*gridBounds.x*gridBounds.y)/gridBounds.x;
    gridPos.x = i - gridPos.z*gridBounds.x*gridBounds.y - gridPos.y*gridBounds.x;
    gridPos.w = 1.0;

    float4 p;
    p.x = (float)gridPos.x;
    p.y = (float)gridPos.y;
    p.z = (float)gridPos.z;
    p.w = (float)gridPos.w;
    
    // calculate cell vertex positions
    float4 v[8];
    v[0] = p;
    v[1] = p + (float4)(1.0f, 0.0f, 0.0f, 0.0f);
    v[2] = p + (float4)(1.0f, 1.0f, 0.0f, 0.0f);
    v[3] = p + (float4)(0.0f, 1.0f, 0.0f, 0.0f);
    v[4] = p + (float4)(0.0f, 0.0f, 1.0f, 0.0f);
    v[5] = p + (float4)(1.0f, 0.0f, 1.0f, 0.0f);
    v[6] = p + (float4)(1.0f, 1.0f, 1.0f, 0.0f);
    v[7] = p + (float4)(0.0f, 1.0f, 1.0f, 0.0f);

    // read field values at neighboring grid vertices
    float field[8];
    field[0] = read_imagef(volume, volumeSampler, gridPos).x;
    field[1] = read_imagef(volume, volumeSampler, gridPos + (int4)(1, 0, 0, 0)).x;
    field[2] = read_imagef(volume, volumeSampler, gridPos + (int4)(1, 1, 0, 0)).x;
    field[3] = read_imagef(volume, volumeSampler, gridPos + (int4)(0, 1, 0, 0)).x;
    field[4] = read_imagef(volume, volumeSampler, gridPos + (int4)(0, 0, 1, 0)).x;
    field[5] = read_imagef(volume, volumeSampler, gridPos + (int4)(1, 0, 1, 0)).x;
    field[6] = read_imagef(volume, volumeSampler, gridPos + (int4)(1, 1, 1, 0)).x;
    field[7] = read_imagef(volume, volumeSampler, gridPos + (int4)(0, 1, 1, 0)).x;
    
    // calculate flag indicating if each vertex is inside or outside isosurface
    int cubeindex;
	cubeindex =  (field[0] < isoValue); 
	cubeindex += (field[1] < isoValue)*2; 
	cubeindex += (field[2] < isoValue)*4; 
	cubeindex += (field[3] < isoValue)*8; 
	cubeindex += (field[4] < isoValue)*16; 
	cubeindex += (field[5] < isoValue)*32; 
	cubeindex += (field[6] < isoValue)*64; 
	cubeindex += (field[7] < isoValue)*128;

	// find the vertices where the surface intersects the cube
	__local float4 vertlist[16*NTHREADS]; //shouldn't this be 12*NTHREADS????????
	vertlist[tid] = vertexInterp(isoValue, v[0], v[1], field[0], field[1]);
    vertlist[NTHREADS+tid] = vertexInterp(isoValue, v[1], v[2], field[1], field[2]);
    vertlist[(NTHREADS*2)+tid] = vertexInterp(isoValue, v[2], v[3], field[2], field[3]);
    vertlist[(NTHREADS*3)+tid] = vertexInterp(isoValue, v[3], v[0], field[3], field[0]);
	vertlist[(NTHREADS*4)+tid] = vertexInterp(isoValue, v[4], v[5], field[4], field[5]);
    vertlist[(NTHREADS*5)+tid] = vertexInterp(isoValue, v[5], v[6], field[5], field[6]);
    vertlist[(NTHREADS*6)+tid] = vertexInterp(isoValue, v[6], v[7], field[6], field[7]);
    vertlist[(NTHREADS*7)+tid] = vertexInterp(isoValue, v[7], v[4], field[7], field[4]);
	vertlist[(NTHREADS*8)+tid] = vertexInterp(isoValue, v[0], v[4], field[0], field[4]);
    vertlist[(NTHREADS*9)+tid] = vertexInterp(isoValue, v[1], v[5], field[1], field[5]);
    vertlist[(NTHREADS*10)+tid] = vertexInterp(isoValue, v[2], v[6], field[2], field[6]);
    vertlist[(NTHREADS*11)+tid] = vertexInterp(isoValue, v[3], v[7], field[3], field[7]);
    barrier(CLK_LOCAL_MEM_FENCE);

    // read number of vertices from texture
    uint numVerts = read_imageui(numVertsTex, tableSampler, (int2)(cubeindex,0)).x;

    // fill trianges array with vertices (set to 0 if non existant)
    for(int j=0; j<MAX_VERTS_PER_CELL; j++) { //go through all possible triangles

        float4 vertex, normal;
        uint edge;

        edge = read_imageui(triTex, tableSampler, (int2)(j,cubeindex)).x;
        if(edge < 12) {
            vertex = vertlist[(edge*NTHREADS)+tid];
            normal = normalize(read_imagef(gradient, gradientSampler, vertex));
        } else {
            vertex = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
            normal = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
        }

        triangles[i*MAX_VERTS_PER_CELL + j] = vertex;
        normals[i*MAX_VERTS_PER_CELL + j] = normal;

    }

}
