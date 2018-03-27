//GLSL#version 150 compatibility
//GLSL#extension GL_EXT_geometry_shader4 : enable
//
// fat sphere test using a geometry shader (rather than vertex shader)

uniform vec4 iris_Viewport;	// corner and size of window

#include <sphere.glsl>		// sphere support functions

uniform float fatpointSize;	// sphere radius
out vec4 center;		// sphere center
out vec2 zrange;		// min and max depth

void main() {
    // convert sphere center back to model space
    vec4 Pc = gl_ProjectionMatrixInverse * gl_PositionIn[0];
    vec4 Po = gl_ModelViewMatrixInverse * Pc;
    vec4 color = gl_FrontColorIn[0];

    // compute bounds
    vec4 ssbb; vec2 zr;
    if (sphereBBox(/* in */ Po, fatpointSize, /* out */ ssbb, zr))
	return;

    // common data passed to all vertices of proxy geometry
    center = Po;
    zrange = zr;
    gl_FrontColor = color;
    gl_ClipVertex = Pc;

    // output four vertices of bounding box
    // depth at far side of sphere so full sphere can be hither clipped
    gl_Position = vec4(ssbb.xy, zr.y, 1.);
    EmitVertex();

    gl_Position = vec4(ssbb.xw, zr.y, 1.);
    EmitVertex();

    gl_Position = vec4(ssbb.zy, zr.y, 1.);
    EmitVertex();

    gl_Position = vec4(ssbb.zw, zr.y, 1.);
    EmitVertex();
}
