// vertex shader for clipped volume rendering box
// uses special geometry with polygons for box and clipping planes
//
// Each geometric component is identified by whether the coordinate along
// certain axes is zero or not.
//
// - corner vertices of the box will be at +/- volumeSize.xyz/2 (x,y,z!=0)
// - clip planes will all be at x=0, with six vertices evenly spaced in y
//   - near plane will also have z=0 (x,z==0, y!=0)
//   - user planes will be evenly spaced in z (x==0, y,z != 0)
// - view-aligned slices at y=0, vertices spaced in x, planes spaced in z 
//     (x!=0, y==0, z=?)
//
// To summarize: where
//    X,Y,Z = box size, 
//    V = coded vertex number, 
//    p,P = coded plane number
// upper case letters are never 0, lower case may sometimes be 0
//                   x  y  z
// box               X  Y  Z
// near clip plane   0  V  0
// user clip planes  0  V  P
// slices            V  0  p
//
// gl_FrontColor output is ignored by shader, but set for debugging
// purposes to: box=red, near=green, user=yellow, slice=blue

#include "uniform.glsl"

out vec4 Eo;		// eye location in object space
out vec4 Vo;		// position in object space
out vec4 DebugColor;	// color each plane for debugging


void main() {
    // compute eye location in object space
    // work around NVIDIA driver bug: gl_ModelViewProjectionMatrixInverse=0
    Eo = gl_ModelViewMatrixInverse*gl_ProjectionMatrixInverse
	*vec4(0.,0.,-1.,0.);

    // default vertex position (works for box, overridden for planes)
    Vo = gl_Vertex;
    DebugColor = vec4(1.,0.,0.,1.);

    // identify and transform clipping planes
    if (gl_Vertex.x == 0. || gl_Vertex.y == 0.) {
	vec4 plane=vec4(0);     // plane coefficients
	int vertNum=0;		// vertex number 0..5

#ifdef NEAR
	// near clipping plane (z=-1 in projection space)
        if (gl_Vertex.x == 0 && gl_Vertex.z == 0.) {
	    vertNum = int(3.+5.*gl_Vertex.y/volumeSize.y);
            plane = vec4(0.,0.,1.,1.)*gl_ModelViewProjectionMatrix;
	    DebugColor = vec4(0.,1.,0.,1.);
	}
#endif

#ifdef CLIP
	// user clipping plane
        if (gl_Vertex.x == 0. && gl_Vertex.z != 0) {
	    vertNum = int(3.+5.*gl_Vertex.y/volumeSize.y);

	    // transform user clip plane from view space back to model space
	    int planeNum = int(gl_MaxClipPlanes*.5+
			       (gl_MaxClipPlanes-1.)*gl_Vertex.z/volumeSize.z);
	    plane = gl_ClipPlane[planeNum]*gl_ModelViewMatrix;

	    DebugColor = vec4(1.,1.,0.,1.);
	}
#endif

#ifdef SLICES
	// slicing plane parallel to screen
        if (gl_Vertex.y == 0) {
	    vertNum = int(3.+5.*gl_Vertex.x/volumeSize.x);

	    // compute plane location based on view direction 
	    // spaced to fill diagonal of box
	    plane.xyz = transpose(gl_ModelViewProjectionMatrix)[2].xyz;
	    plane.w = length(plane.xyz)*length(volumeSize)
		*gl_Vertex.z/volumeSize.z;

	    DebugColor = vec4(0.,0.,1.,1.);
	}
#endif

	// bump just a little inside, so won't be clipped by real plane
	plane.w -= 1e-5;

	//// compute intersection of plane and box, based on method by 
	// Engel et al., Real-Time Volume Graphics, AK Peters 2006
        //   A single plane can intersect a box in up to six places.
        //   Geometry is a six-vertex tristrip zigzagging    2---3
        //   to make a hexagon. (012) triangle should be    /|  /|\
        //   front-facing. Between front and 2nd vertex    0 | / | 5
        //   01, 23 and 45 coincide and some tris are       \|/  |/
        //   degenerate. Between 2nd and 3rd, each is        1---4
        //   unique. Between 3rd vertex and back, 02, 14 and 35 coincide.

	// Front is first corner relative the plane normal
	vec3 front = (.5-step(vec3(0.),plane.xyz))*volumeSize;

	// ensure front facing triangles in plane
	vec3 sgn = sign(front);
	int flip = sgn.x*sgn.y*sgn.z < 0. ? 1 : 0;

	// find four corners from front to back
	vec4 v0 = vec4(front,1.), v1, v2, v3 = vec4(-front,1.);

        // for v1, six verts flip signs for xxyzyz relative to v0
        // for v2, flip relative to v1:     yzxxzy
	if (vertNum == 0) {
	  v1 = vec4(-v0.x, v0.y, v0.z, 1.); v2 = vec4(-v0.x,-v0.y, v0.z, 1.);
        } 
	else if (vertNum == 1+flip) {
	  v1 = vec4(-v0.x, v0.y, v0.z, 1.); v2 = vec4(-v0.x, v0.y,-v0.z, 1.);
        } 
	else if (vertNum == 2-flip) {
	  v1 = vec4( v0.x,-v0.y, v0.z, 1.); v2 = vec4(-v0.x,-v0.y, v0.z, 1.);
        } 
	else if (vertNum == 3+flip) {
	  v1 = vec4( v0.x, v0.y,-v0.z, 1.); v2 = vec4(-v0.x, v0.y,-v0.z, 1.);
        } 
	else if (vertNum == 4-flip) {
	  v1 = vec4( v0.x,-v0.y, v0.z, 1.); v2 = vec4( v0.x,-v0.y,-v0.z, 1.);
        }
	else {
	  v1 = vec4( v0.x, v0.y,-v0.z, 1.); v2 = vec4( v0.x,-v0.y,-v0.z, 1.);
        }

	// plane equations for each vertex against the clipping plane
	float d0 = dot(plane,v0), d1 = dot(plane,v1);
	float d2 = dot(plane,v2), d3 = dot(plane,v3);

	// find intersection edge
	if (d0>0)
	    Vo = v0;
	else if (d1>0)
	    Vo = vec4(v0.xyz*v1.w*d1 - v1.xyz*v0.w*d0, (d1-d0)*v0.w*v1.w);
	else if (d2>0)
	    Vo = vec4(v1.xyz*v2.w*d2 - v2.xyz*v1.w*d1, (d2-d1)*v1.w*v2.w);
	else if (d3>0)
	    Vo = vec4(v2.xyz*v3.w*d3 - v3.xyz*v2.w*d2, (d3-d2)*v2.w*v3.w);
	else
	    Vo = v3;
    }

    gl_ClipVertex = gl_ModelViewMatrix * Vo;
    gl_Position = gl_ModelViewProjectionMatrix * Vo;
}

