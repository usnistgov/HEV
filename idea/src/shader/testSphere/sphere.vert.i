//GLSL#version 150 compatibility

uniform vec4 iris_Viewport;	// corner and size of window

#include <sphere.glsl>		// sphere support functions

uniform float fatpointSize;	// sphere radius
out vec4 center;		// sphere center
out vec2 zrange;		// min and max depth

void main() {
    // base color
    gl_FrontColor = gl_Color;

    // center and radius
    center = gl_Vertex;

    // compute sphere
    sphereSize(/* in */ center, fatpointSize, 
	       /* out */ zrange, gl_Position, gl_PointSize);
}















