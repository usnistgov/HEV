//GLSL#version 150 compatibility
//GLSL#extension GL_EXT_geometry_shader4 : enable
//
// Geometry shader for lit spheres

uniform vec4 iris_Viewport;	// corner and size of window

#include <sphere.glsl>		// sphere support functions

uniform float fatpointSize = -1; // sphere radius (-1 = radius from alpha)

out vec4 center;		// sphere center
out vec2 zrange;		// min and max depth
out float aradius;              // when radius passed in via alpha, pass it
                                // to frag shader.

void main() {
    // convert sphere center back to model space
    vec4 Pc = gl_ProjectionMatrixInverse * gl_PositionIn[0];
    vec4 Po = gl_ModelViewMatrixInverse * Pc;
    vec4 color = gl_FrontColorIn[0];

    // select radius from uniform or alpha
    float r;
    if (fatpointSize < 0)
      r = color[3];
    else
      r= fatpointSize;

    // compute bounds
    vec4 ssbb; vec2 zr;
    if (sphereBBox(/* in */ Po, r, /* out */ ssbb, zr))
	return;

    // common data passed to all vertices of proxy geometry
    center = Po;
    zrange = zr;
    aradius = r;
    gl_FrontColor = color;
    gl_ClipVertex = Pc;

    // output four vertices of bounding box
    gl_Position = vec4(ssbb.xy, zr.y, 1.);
    EmitVertex();

    gl_Position = vec4(ssbb.xw, zr.y, 1.);
    EmitVertex();

    gl_Position = vec4(ssbb.zy, zr.y, 1.);
    EmitVertex();

    gl_Position = vec4(ssbb.zw, zr.y, 1.);
    EmitVertex();
}
